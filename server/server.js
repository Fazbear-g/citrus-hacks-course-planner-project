// ============================================================
// server.js — UCR Course Planner Middleware
// Features:
//   - UCR NetID style login/register with bcrypt + JWT
//   - C++ planner integration
//   - Gemini AI review
//   - Per-student taken courses via student_courses table
//
// Usage: node server.js
// ============================================================

const express              = require('express');
const cors                 = require('cors');
const { execFile }         = require('child_process');
const path                 = require('path');
const sqlite3              = require('better-sqlite3');
const bcrypt               = require('bcrypt');
const jwt                  = require('jsonwebtoken');
const { GoogleGenerativeAI } = require('@google/generative-ai');

const app    = express();
const genAI  = new GoogleGenerativeAI(process.env.GEMINI_API_KEY || '');
const JWT_SECRET = process.env.JWT_SECRET || 'ucr-course-planner-secret-2024';
const SALT_ROUNDS = 10;

app.use(cors());
app.use(express.json());

// ── Database setup ────────────────────────────────────────────────────────────

const DB_PATH     = path.join(__dirname, '..', 'courses.db');
const BINARY_PATH = path.join(__dirname, '..', 'courses_db');

const db = new sqlite3(DB_PATH);

// Create students table if it doesn't exist
db.exec(`
    CREATE TABLE IF NOT EXISTS students (
        id          INTEGER PRIMARY KEY AUTOINCREMENT,
        net_id      TEXT NOT NULL UNIQUE,
        password    TEXT NOT NULL,
        full_name   TEXT,
        major       TEXT,
        created_at  TEXT DEFAULT (datetime('now'))
    );

    CREATE TABLE IF NOT EXISTS student_courses (
        id          INTEGER PRIMARY KEY AUTOINCREMENT,
        student_id  TEXT NOT NULL,
        course_id   TEXT NOT NULL,
        taken       INTEGER NOT NULL DEFAULT 1,
        UNIQUE(student_id, course_id)
    );
`);

// ── Auth Middleware ───────────────────────────────────────────────────────────

function requireAuth(req, res, next) {
    const header = req.headers.authorization;
    if (!header || !header.startsWith('Bearer ')) {
        return res.status(401).json({ error: 'Unauthorized' });
    }
    const token = header.split(' ')[1];
    try {
        const decoded = jwt.verify(token, JWT_SECRET);
        req.user = decoded;
        next();
    } catch {
        return res.status(401).json({ error: 'Invalid or expired token' });
    }
}

// ── Valid majors ──────────────────────────────────────────────────────────────

const VALID_MAJORS = {
    'CS':    'Computer Science',
    'CE':    'Computer Engineering',
    'EE':    'Electrical Engineering',
    'ME':    'Mechanical Engineering',
    'CHEME': 'Chemical Engineering',
    'BIOE':  'Bioengineering',
    'ENVE':  'Environmental Engineering',
    'MSE':   'Materials Science & Engineering',
    'DS':    'Data Science',
    'ROBO':  'Robotics Engineering'
};

// ── POST /api/register ────────────────────────────────────────────────────────
// Body: { netId, password, fullName, major }

app.post('/api/register', async (req, res) => {
    const { netId, password, fullName, major } = req.body;

    if (!netId || !password) {
        return res.status(400).json({ error: 'NetID and password are required' });
    }

    // Validate NetID format (letters + numbers, like jdoe001)
    if (!/^[a-z][a-z0-9]{2,}$/.test(netId.toLowerCase())) {
        return res.status(400).json({ error: 'Invalid NetID format. Use lowercase letters and numbers (e.g. jdoe001)' });
    }

    if (password.length < 8) {
        return res.status(400).json({ error: 'Password must be at least 8 characters' });
    }

    // Check if NetID already exists
    const existing = db.prepare('SELECT id FROM students WHERE net_id = ?').get(netId.toLowerCase());
    if (existing) {
        return res.status(409).json({ error: 'This NetID is already registered' });
    }

    try {
        const hashed = await bcrypt.hash(password, SALT_ROUNDS);
        db.prepare('INSERT INTO students (net_id, password, full_name, major) VALUES (?, ?, ?, ?)').run(
            netId.toLowerCase(), hashed, fullName || '', major || ''
        );

        const token = jwt.sign({ netId: netId.toLowerCase(), fullName: fullName || '' }, JWT_SECRET, { expiresIn: '7d' });
        res.json({ token, netId: netId.toLowerCase(), fullName: fullName || '', major: major || '' });
    } catch (err) {
        res.status(500).json({ error: 'Registration failed' });
    }
});

// ── POST /api/login ───────────────────────────────────────────────────────────
// Body: { netId, password }

app.post('/api/login', async (req, res) => {
    const { netId, password } = req.body;

    if (!netId || !password) {
        return res.status(400).json({ error: 'NetID and password are required' });
    }

    const student = db.prepare('SELECT * FROM students WHERE net_id = ?').get(netId.toLowerCase());
    if (!student) {
        return res.status(401).json({ error: 'Invalid NetID or password' });
    }

    const match = await bcrypt.compare(password, student.password);
    if (!match) {
        return res.status(401).json({ error: 'Invalid NetID or password' });
    }

    const token = jwt.sign(
        { netId: student.net_id, fullName: student.full_name },
        JWT_SECRET,
        { expiresIn: '7d' }
    );

    res.json({
        token,
        netId:    student.net_id,
        fullName: student.full_name,
        major:    student.major,
    });
});

// ── GET /api/me ───────────────────────────────────────────────────────────────

app.get('/api/me', requireAuth, (req, res) => {
    const student = db.prepare('SELECT net_id, full_name, major FROM students WHERE net_id = ?').get(req.user.netId);
    if (!student) return res.status(404).json({ error: 'Student not found' });

    // Also get their taken courses
    const taken = db.prepare('SELECT course_id FROM student_courses WHERE student_id = ? AND taken = 1').all(req.user.netId);

    res.json({
        netId:    student.net_id,
        fullName: student.full_name,
        major:    student.major,
        takenCourses: taken.map(r => r.course_id),
    });
});

// ── PUT /api/me/taken ─────────────────────────────────────────────────────────
// Body: { takenCourses: ["CS010A", "MATH009A"] }
// Saves taken courses for the logged-in student

app.put('/api/me/taken', requireAuth, (req, res) => {
    const { takenCourses = [] } = req.body;
    const netId = req.user.netId;

    // Clear existing taken courses for this student
    db.prepare('DELETE FROM student_courses WHERE student_id = ?').run(netId);

    // Insert new ones
    const insert = db.prepare('INSERT OR IGNORE INTO student_courses (student_id, course_id, taken) VALUES (?, ?, 1)');
    for (const courseId of takenCourses) {
        if (courseId) insert.run(netId, courseId.toUpperCase());
    }

    res.json({ success: true, takenCourses });
});

// ── GET /api/majors ───────────────────────────────────────────────────────────

app.get('/api/majors', (req, res) => {
    const majors = Object.entries(VALID_MAJORS).map(([code, name]) => ({ code, name }));
    res.json(majors);
});

// ── POST /api/plan ────────────────────────────────────────────────────────────
// Body: { major, takenCourses }
// Auth optional — if logged in uses student's saved courses

app.post('/api/plan', async (req, res) => {
    let { major, takenCourses = [] } = req.body;

    // If auth header present, load saved courses for this student
    const header = req.headers.authorization;
    if (header?.startsWith('Bearer ')) {
        try {
            const decoded = jwt.verify(header.split(' ')[1], JWT_SECRET);
            const saved   = db.prepare('SELECT course_id FROM student_courses WHERE student_id = ? AND taken = 1').all(decoded.netId);
            if (saved.length > 0) takenCourses = saved.map(r => r.course_id);

            // Also get their major if not provided
            if (!major) {
                const student = db.prepare('SELECT major FROM students WHERE net_id = ?').get(decoded.netId);
                if (student?.major) major = student.major;
            }
        } catch { /* token invalid, proceed as guest */ }
    }

    const majorUpper = (major || '').toUpperCase();
    if (!VALID_MAJORS[majorUpper]) {
        return res.status(400).json({ error: `Invalid major. Valid options: ${Object.keys(VALID_MAJORS).join(', ')}` });
    }

    const takenArg = takenCourses.join(',');

    // Run C++ planner
    let rawPlan;
    try {
        rawPlan = await runPlanner(majorUpper, takenArg);
    } catch (err) {
        return res.status(500).json({ error: `Planner error: ${err.message}` });
    }

    // Gemini review (optional)
    let aiReview = null;
    if (process.env.GEMINI_API_KEY) {
        try {
            aiReview = await reviewWithGemini(rawPlan, majorUpper, VALID_MAJORS[majorUpper], takenCourses);
        } catch (err) {
            console.error('Gemini error:', err.message);
        }
    }

    res.json({ majorName: VALID_MAJORS[majorUpper], major: majorUpper, rawPlan, aiReview });
});

// ── Helpers ───────────────────────────────────────────────────────────────────

function runPlanner(major, takenCourses) {
    return new Promise((resolve, reject) => {
        const args = [major];
        if (takenCourses) args.push(takenCourses);
        execFile(BINARY_PATH, args, { cwd: path.join(__dirname, '..') }, (err, stdout, stderr) => {
            if (err) { reject(new Error(stderr || err.message)); return; }
            resolve(stdout);
        });
    });
}

async function reviewWithGemini(rawPlan, majorCode, majorName, takenCourses) {
    const takenList = takenCourses.length > 0
        ? `The student has already completed: ${takenCourses.join(', ')}.`
        : 'The student has not completed any courses yet.';

    const prompt = `You are an academic advisor at UC Riverside helping a ${majorName} student plan their courses.

Here is a computer-generated course plan:
${rawPlan}
${takenList}

Please review this plan and provide:
1. A brief assessment (2-3 sentences)
2. Any quarters that look too difficult or too light
3. Any sequencing concerns
4. 2-3 concrete suggestions

Keep it concise and practical.`;

    const model  = genAI.getGenerativeModel({ model: 'gemini-1.5-flash' });
    const result = await model.generateContent(prompt);
    return result.response.text();
}

// ── Start ─────────────────────────────────────────────────────────────────────


app.get('/api/prereqs', (req, res) => {
    const rows = db.prepare('SELECT course_id, prerequisite FROM courses WHERE prerequisite IS NOT NULL').all();
    
    // Build map: { courseId: { prereq, unlocks[] } }
    const map = {};
    for (const row of rows) {
        // This course has a prereq
        if (!map[row.course_id]) map[row.course_id] = { prereq: row.prerequisite, unlocks: [] };
        else map[row.course_id].prereq = row.prerequisite;

        // The prereq unlocks this course
        if (!map[row.prerequisite]) map[row.prerequisite] = { prereq: null, unlocks: [row.course_id] };
        else map[row.prerequisite].unlocks.push(row.course_id);
    }
    res.json(map);
});

const PORT = process.env.PORT || 3001;

app.listen(PORT, () => {
    console.log(`\n🚀 UCR Course Planner server running on http://localhost:${PORT}`);
    console.log(`   POST /api/register  — create account`);
    console.log(`   POST /api/login     — login`);
    console.log(`   GET  /api/me        — get student profile`);
    console.log(`   PUT  /api/me/taken  — save taken courses`);
    console.log(`   POST /api/plan      — generate plan\n`);
});