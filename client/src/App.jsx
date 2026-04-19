import { useState, useMemo, useEffect } from "react";

// ── Constants ─────────────────────────────────────────────────────────────────

const MAJORS = [
  { code: "CS",    name: "Computer Science" },
  { code: "CE",    name: "Computer Engineering" },
  { code: "EE",    name: "Electrical Engineering" },
  { code: "ME",    name: "Mechanical Engineering" },
  { code: "CHEME", name: "Chemical Engineering" },
  { code: "BIOE",  name: "Bioengineering" },
  { code: "ENVE",  name: "Environmental Engineering" },
  { code: "MSE",   name: "Materials Science & Engineering" },
  { code: "DS",    name: "Data Science" },
  { code: "ROBO",  name: "Robotics Engineering" },
];

const SEASONS     = ["Fall", "Winter", "Spring"];
const TOTAL_UNITS = 180;
const API         = "http://localhost:3001";

// ── Helpers ───────────────────────────────────────────────────────────────────

function quarterLabel(n) {
  const idx = n - 1;
  return { year: Math.floor(idx / 3) + 1, season: SEASONS[idx % 3], label: `Year ${Math.floor(idx / 3) + 1} — ${SEASONS[idx % 3]}` };
}

const diffColor = (s) => s <= 3 ? "#4ade80" : s <= 5 ? "#facc15" : s <= 7 ? "#fb923c" : "#f87171";
const diffLabel = (s) => s <= 3 ? "Easy" : s <= 5 ? "Moderate" : s <= 7 ? "Hard" : "Very Hard";

function parsePlan(raw) {
  const quarters = [], completed = [];
  const lines = raw.split("\n");
  let cur = null, inComp = false, totalUnits = 0;
  for (const line of lines) {
    if (line.includes("Already Completed")) { inComp = true; continue; }
    if (line.match(/^── Quarter \d+/)) {
      inComp = false;
      const m = line.match(/Quarter (\d+)\s+\[(\d+) units \| avg difficulty: ([\d.]+)/);
      if (m) { cur = { number: +m[1], units: +m[2], avgDifficulty: +m[3], courses: [] }; quarters.push(cur); totalUnits += +m[2]; }
      continue;
    }
    const cm = line.match(/^\s{2}(\S+)\s+\|\s+(.+?)\s+\|\s+(\d+) units\s+\|\s+diff:\s+([\d.]+)(.*)/);
    if (cm) {
      const c = { id: cm[1].trim(), name: cm[2].trim(), units: +cm[3], difficulty: +cm[4], juniorPlus: cm[5].includes("Junior+"), isBreadth: cm[1].includes("BREADTH") };
      if (inComp) completed.push(c); else if (cur) cur.courses.push(c);
      continue;
    }
    const dm = line.match(/✓\s+(\S+)\s+\|\s+(.+?)\s+\((\d+) units\)/);
    if (dm && inComp) completed.push({ id: dm[1].trim(), name: dm[2].trim(), units: +dm[3], difficulty: 0, juniorPlus: false, isBreadth: false });
  }
  return { quarters, completed, totalUnits };
}

function buildCourseIndex(plan) {
  const index = {};
  for (const q of plan.quarters) for (const c of q.courses) index[c.id] = { ...c, quarterNumber: q.number };
  for (const c of plan.completed) index[c.id] = { ...c, quarterNumber: 0 };
  return index;
}


function getChain(courseId, map) {
  const ancestors = [], descendants = [];
  let cur = courseId;
  const visited = new Set();
  while (map[cur]?.prereq && !visited.has(cur)) {
    visited.add(cur); ancestors.unshift(map[cur].prereq); cur = map[cur].prereq;
  }
  const queue = [...(map[courseId]?.unlocks ?? [])], fv = new Set();
  while (queue.length) {
    const next = queue.shift(); if (fv.has(next)) continue; fv.add(next); descendants.push(next);
    (map[next]?.unlocks ?? []).forEach(u => queue.push(u));
  }
  return { ancestors, descendants };
}

// ── UI Components ─────────────────────────────────────────────────────────────

const DiffBar = ({ score, width = 60 }) => (
  <div style={{ display: "flex", alignItems: "center", gap: "6px" }}>
    <div style={{ width, height: "4px", borderRadius: "2px", background: "#1e2433", overflow: "hidden" }}>
      <div style={{ width: `${(score / 10) * 100}%`, height: "100%", borderRadius: "2px", background: diffColor(score) }} />
    </div>
    <span style={{ fontSize: "10px", color: "#64748b", fontFamily: "monospace" }}>{score.toFixed(1)}</span>
  </div>
);

function ChainViz({ courseId, courseIndex, prereqMap }) {
  const { ancestors, descendants } = getChain(courseId, prereqMap);
  if (!ancestors.length && !descendants.length) return <div style={{ fontSize: "11px", color: "#334155", textAlign: "center", padding: "8px 0" }}>No chain data available</div>;
  const Node = ({ id, highlight }) => {
    const c = courseIndex[id];
    return (
      <div style={{ background: highlight ? "#0f2040" : "#0d1220", border: `1px solid ${highlight ? "#3b82f6" : "#1f2937"}`, borderRadius: "8px", padding: "6px 12px", fontSize: "11px", fontFamily: "monospace", color: highlight ? "#93c5fd" : "#e2e8f0", whiteSpace: "nowrap", boxShadow: highlight ? "0 0 12px rgba(59,130,246,0.25)" : "none" }}>
        {id}
        {c && <div style={{ fontSize: "9px", color: "#475569", marginTop: "1px" }}>{c.name?.slice(0, 18)}{c.name?.length > 18 ? "…" : ""}</div>}
      </div>
    );
  };
  return (
    <div style={{ overflowX: "auto", paddingBottom: "4px" }}>
      <div style={{ display: "flex", alignItems: "center", gap: "4px", minWidth: "max-content" }}>
        {ancestors.map(id => <div key={id} style={{ display: "flex", alignItems: "center", gap: "4px" }}><Node id={id} /><span style={{ color: "#334155", fontSize: "14px" }}>→</span></div>)}
        <Node id={courseId} highlight />
        {descendants.map((id, i) => <div key={id} style={{ display: "flex", alignItems: "center", gap: "4px" }}><span style={{ color: i === 0 ? "#3b82f6" : "#334155", fontSize: "14px" }}>→</span><Node id={id} /></div>)}
      </div>
    </div>
  );
}

function CourseModal({ course, qlabel, courseIndex, prereqMap, onClose }) {
  if (!course) return null;
  const isUpper = course.id.match(/\d+/) && parseInt(course.id.match(/\d+/)[0]) >= 100;
  return (
    <div onClick={onClose} style={{ position: "fixed", inset: 0, background: "rgba(0,0,0,0.8)", display: "flex", alignItems: "center", justifyContent: "center", zIndex: 1000, backdropFilter: "blur(6px)" }}>
      <div onClick={e => e.stopPropagation()} style={{ background: "#0a0e1a", border: "1px solid #1e2d40", borderRadius: "16px", padding: "28px", width: "min(520px, 92vw)", maxHeight: "90vh", overflowY: "auto", boxShadow: "0 30px 80px rgba(0,0,0,0.7)" }}>
        <div style={{ display: "flex", justifyContent: "space-between", alignItems: "flex-start", marginBottom: "20px" }}>
          <div>
            <div style={{ display: "flex", alignItems: "center", gap: "8px" }}>
              <span style={{ fontSize: "22px", fontWeight: "800", color: "#f1f5f9", fontFamily: "monospace" }}>{course.id}</span>
              {course.juniorPlus && <span style={{ fontSize: "9px", background: "#1e1b4b", color: "#a5b4fc", padding: "2px 7px", borderRadius: "20px", border: "1px solid #4338ca" }}>JUNIOR+</span>}
              {course.isBreadth && <span style={{ fontSize: "9px", background: "#0f2a4a", color: "#60a5fa", padding: "2px 7px", borderRadius: "20px", border: "1px solid #1e4a8a" }}>BREADTH</span>}
            </div>
            <div style={{ fontSize: "12px", color: "#64748b", marginTop: "4px" }}>{course.name}</div>
          </div>
          <button onClick={onClose} style={{ background: "none", border: "none", color: "#475569", fontSize: "18px", cursor: "pointer" }}>✕</button>
        </div>
        <div style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: "10px", marginBottom: "14px" }}>
          {[["Units", `${course.units} units`], ["Quarter", qlabel], ["Division", isUpper ? "Upper Division" : "Lower Division"], ["Standing", course.juniorPlus ? "Junior+ Required" : "Open Enrollment"]].map(([l, v]) => (
            <div key={l} style={{ background: "#0d1220", borderRadius: "8px", padding: "10px 14px", border: "1px solid #141e30" }}>
              <div style={{ fontSize: "9px", color: "#334155", letterSpacing: "0.12em", marginBottom: "3px" }}>{l.toUpperCase()}</div>
              <div style={{ fontSize: "12px", color: "#e2e8f0" }}>{v}</div>
            </div>
          ))}
        </div>
        <div style={{ background: "#0d1220", borderRadius: "10px", padding: "14px", border: "1px solid #141e30", marginBottom: "14px" }}>
          <div style={{ display: "flex", justifyContent: "space-between", marginBottom: "8px" }}>
            <div style={{ fontSize: "9px", color: "#334155", letterSpacing: "0.12em" }}>DIFFICULTY</div>
            <span style={{ fontSize: "9px", color: diffColor(course.difficulty) }}>{diffLabel(course.difficulty)}</span>
          </div>
          <div style={{ display: "flex", alignItems: "center", gap: "12px" }}>
            <div style={{ flex: 1, height: "7px", borderRadius: "4px", background: "#111827", overflow: "hidden" }}>
              <div style={{ width: `${(course.difficulty / 10) * 100}%`, height: "100%", borderRadius: "4px", background: diffColor(course.difficulty) }} />
            </div>
            <span style={{ fontSize: "20px", fontWeight: "700", color: diffColor(course.difficulty), fontFamily: "monospace" }}>{course.difficulty.toFixed(1)}</span>
          </div>
        </div>
        <div style={{ background: "#0d1220", borderRadius: "10px", padding: "14px", border: "1px solid #141e30" }}>
          <div style={{ fontSize: "9px", color: "#334155", letterSpacing: "0.12em", marginBottom: "10px" }}>PREREQUISITE CHAIN</div>
          <ChainViz courseId={course.id} courseIndex={courseIndex} prereqMap={prereqMap} />
        </div>
      </div>
    </div>
  );
}

function SearchBar({ courseIndex, onSelect }) {
  const [query, setQuery] = useState("");
  const [open, setOpen]   = useState(false);
  const results = useMemo(() => {
    if (!query || query.length < 2) return [];
    const q = query.toLowerCase();
    return Object.values(courseIndex).filter(c => c.id.toLowerCase().includes(q) || c.name.toLowerCase().includes(q)).slice(0, 8);
  }, [query, courseIndex]);
  return (
    <div style={{ position: "relative" }}>
      <input value={query} onChange={e => { setQuery(e.target.value); setOpen(true); }} onFocus={() => setOpen(true)} onBlur={() => setTimeout(() => setOpen(false), 150)} placeholder="Search courses… (e.g. CS153, Operating Systems)" style={{ width: "100%", padding: "9px 13px", background: "#0d1220", border: "1px solid #141e30", borderRadius: "8px", color: "#f1f5f9", fontSize: "12px", fontFamily: "inherit", outline: "none", boxSizing: "border-box" }} />
      {open && results.length > 0 && (
        <div style={{ position: "absolute", top: "calc(100% + 6px)", left: 0, right: 0, background: "#0a0e1a", border: "1px solid #1e2d40", borderRadius: "10px", overflow: "hidden", zIndex: 100, boxShadow: "0 16px 40px rgba(0,0,0,0.6)" }}>
          {results.map(c => (
            <div key={c.id} onMouseDown={() => { onSelect(c); setQuery(""); setOpen(false); }} style={{ padding: "9px 13px", cursor: "pointer", borderBottom: "1px solid #0d1220", display: "flex", justifyContent: "space-between", alignItems: "center" }} onMouseEnter={e => e.currentTarget.style.background = "#0d1220"} onMouseLeave={e => e.currentTarget.style.background = "transparent"}>
              <div>
                <span style={{ fontSize: "11px", fontWeight: "700", color: "#f1f5f9", fontFamily: "monospace" }}>{c.id}</span>
                <span style={{ fontSize: "10px", color: "#475569", marginLeft: "10px" }}>{c.name}</span>
              </div>
              <DiffBar score={c.difficulty} width={36} />
            </div>
          ))}
        </div>
      )}
    </div>
  );
}

// ── Auth Pages ────────────────────────────────────────────────────────────────

function AuthPage({ onLogin }) {
  const [mode, setMode]         = useState("login"); // "login" | "register"
  const [netId, setNetId]       = useState("");
  const [password, setPassword] = useState("");
  const [fullName, setFullName] = useState("");
  const [major, setMajor]       = useState("");
  const [error, setError]       = useState("");
  const [loading, setLoading]   = useState(false);

  const handleSubmit = async () => {
    setError(""); setLoading(true);
    try {
      const endpoint = mode === "login" ? "/api/login" : "/api/register";
      const body     = mode === "login" ? { netId, password } : { netId, password, fullName, major };
      const res      = await fetch(`${API}${endpoint}`, { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify(body) });
      const data     = await res.json();
      if (!res.ok) { setError(data.error || "Something went wrong"); return; }
      localStorage.setItem("ucr_token", data.token);
      localStorage.setItem("ucr_user", JSON.stringify({ netId: data.netId, fullName: data.fullName, major: data.major }));
      onLogin(data);
    } catch { setError("Could not connect to the server."); }
    finally { setLoading(false); }
  };

  return (
    <div style={{ minHeight: "100vh", background: "#080c18", display: "flex", flexDirection: "column", alignItems: "center", justifyContent: "center", fontFamily: "'IBM Plex Mono', 'Courier New', monospace", padding: "20px" }}>
      {/* UCR Logo area */}
      <div style={{ textAlign: "center", marginBottom: "36px" }}>
        <div style={{ width: "56px", height: "56px", borderRadius: "14px", background: "linear-gradient(135deg, #2563eb, #7c3aed)", display: "flex", alignItems: "center", justifyContent: "center", fontSize: "24px", fontWeight: "800", color: "white", margin: "0 auto 16px" }}>R</div>
        <div style={{ fontSize: "18px", fontWeight: "700", color: "#f1f5f9", letterSpacing: "0.04em" }}>UCR Course Planner</div>
        <div style={{ fontSize: "10px", color: "#334155", letterSpacing: "0.14em", marginTop: "4px" }}>BOURNS COLLEGE OF ENGINEERING</div>
      </div>

      {/* Card */}
      <div style={{ background: "#0a0e1a", border: "1px solid #141e30", borderRadius: "16px", padding: "32px", width: "min(420px, 100%)", boxShadow: "0 24px 60px rgba(0,0,0,0.5)" }}>
        {/* Tab toggle */}
        <div style={{ display: "flex", background: "#080c18", borderRadius: "8px", padding: "3px", marginBottom: "24px" }}>
          {["login", "register"].map(m => (
            <button key={m} onClick={() => { setMode(m); setError(""); }} style={{ flex: 1, padding: "7px", background: mode === m ? "#141e30" : "transparent", border: "none", borderRadius: "6px", color: mode === m ? "#f1f5f9" : "#475569", fontSize: "11px", fontFamily: "inherit", fontWeight: mode === m ? "700" : "400", letterSpacing: "0.1em", cursor: "pointer", textTransform: "uppercase" }}>
              {m === "login" ? "Sign In" : "Create Account"}
            </button>
          ))}
        </div>

        {/* Fields */}
        <div style={{ display: "flex", flexDirection: "column", gap: "14px" }}>
          {mode === "register" && (
            <div>
              <label style={{ display: "block", fontSize: "9px", color: "#475569", letterSpacing: "0.14em", marginBottom: "6px" }}>FULL NAME</label>
              <input value={fullName} onChange={e => setFullName(e.target.value)} placeholder="Jane Doe" style={{ width: "100%", padding: "9px 12px", background: "#0d1220", border: "1px solid #141e30", borderRadius: "8px", color: "#f1f5f9", fontSize: "12px", fontFamily: "inherit", outline: "none", boxSizing: "border-box" }} />
            </div>
          )}

          <div>
            <label style={{ display: "block", fontSize: "9px", color: "#475569", letterSpacing: "0.14em", marginBottom: "6px" }}>UCR NET ID</label>
            <input value={netId} onChange={e => setNetId(e.target.value.toLowerCase())} placeholder="jdoe001" style={{ width: "100%", padding: "9px 12px", background: "#0d1220", border: "1px solid #141e30", borderRadius: "8px", color: "#f1f5f9", fontSize: "12px", fontFamily: "inherit", outline: "none", boxSizing: "border-box" }} />
          </div>

          <div>
            <label style={{ display: "block", fontSize: "9px", color: "#475569", letterSpacing: "0.14em", marginBottom: "6px" }}>PASSWORD</label>
            <input type="password" value={password} onChange={e => setPassword(e.target.value)} placeholder={mode === "register" ? "Min. 8 characters" : "••••••••"} onKeyDown={e => e.key === "Enter" && handleSubmit()} style={{ width: "100%", padding: "9px 12px", background: "#0d1220", border: "1px solid #141e30", borderRadius: "8px", color: "#f1f5f9", fontSize: "12px", fontFamily: "inherit", outline: "none", boxSizing: "border-box" }} />
          </div>

          {mode === "register" && (
            <div>
              <label style={{ display: "block", fontSize: "9px", color: "#475569", letterSpacing: "0.14em", marginBottom: "6px" }}>MAJOR (OPTIONAL)</label>
              <select value={major} onChange={e => setMajor(e.target.value)} style={{ width: "100%", padding: "9px 12px", background: "#0d1220", border: "1px solid #141e30", borderRadius: "8px", color: major ? "#f1f5f9" : "#475569", fontSize: "12px", fontFamily: "inherit", outline: "none", cursor: "pointer" }}>
                <option value="">Select your major...</option>
                {MAJORS.map(m => <option key={m.code} value={m.code}>{m.name}</option>)}
              </select>
            </div>
          )}
        </div>

        {error && <div style={{ background: "#1a0808", border: "1px solid #7f1d1d", borderRadius: "8px", padding: "9px 13px", fontSize: "11px", color: "#fca5a5", marginTop: "14px" }}>{error}</div>}

        <button onClick={handleSubmit} disabled={loading} style={{ width: "100%", marginTop: "20px", padding: "11px", background: loading ? "#141e30" : "linear-gradient(135deg, #2563eb, #7c3aed)", border: "none", borderRadius: "9px", color: loading ? "#334155" : "white", fontSize: "12px", fontFamily: "inherit", fontWeight: "700", letterSpacing: "0.08em", cursor: loading ? "not-allowed" : "pointer" }}>
          {loading ? "..." : mode === "login" ? "SIGN IN →" : "CREATE ACCOUNT →"}
        </button>
      </div>

      <div style={{ fontSize: "10px", color: "#1e2d40", marginTop: "20px" }}>
        This is not affiliated with UCR's official systems
      </div>
    </div>
  );
}

// ── Main Planner ──────────────────────────────────────────────────────────────

function Planner({ user, onLogout }) {
  const [major, setMajor]         = useState(user.major || "");
  const [takenInput, setTakenInput] = useState("");
  const [plan, setPlan]           = useState(null);
  const [loading, setLoading]     = useState(false);
  const [error, setError]         = useState("");
  const [modal, setModal]         = useState(null);
  const [savingTaken, setSavingTaken] = useState(false);
  const [prereqMap, setPrereqMap] = useState({});
  const [aiReview, setAiReview] = useState(null);

  useEffect(() => {
      fetch(`${API}/api/prereqs`)
          .then(r => r.json())
          .then(data => setPrereqMap(data))
          .catch(() => {});
  }, []);

  const courseIndex    = useMemo(() => plan ? buildCourseIndex(plan) : {}, [plan]);
  const completedUnits = plan?.completed.reduce((a, c) => a + c.units, 0) ?? 0;
  const remainingUnits = plan?.totalUnits ?? 0;
  const progressPct    = Math.min(100, Math.round((completedUnits / TOTAL_UNITS) * 100));
  const plannedPct     = Math.min(100 - progressPct, Math.round((remainingUnits / TOTAL_UNITS) * 100));
  const token          = localStorage.getItem("ucr_token");

  // Load saved taken courses on mount
  useEffect(() => {
    if (!token) return;
    fetch(`${API}/api/me`, { headers: { Authorization: `Bearer ${token}` } })
      .then(r => r.json())
      .then(d => {
        if (d.takenCourses?.length) setTakenInput(d.takenCourses.join(", "));
        if (d.major) setMajor(d.major);
      }).catch(() => {});
  }, []);

  const saveTakenCourses = async () => {
    if (!token) return;
    setSavingTaken(true);
    const takenCourses = takenInput.split(",").map(s => s.trim().toUpperCase()).filter(Boolean);
    await fetch(`${API}/api/me/taken`, {
      method: "PUT", headers: { "Content-Type": "application/json", Authorization: `Bearer ${token}` },
      body: JSON.stringify({ takenCourses }),
    }).catch(() => {});
    setSavingTaken(false);
  };

  const handleGenerate = async () => {
    if (!major) { setError("Please select a major."); return; }
    setError(""); setLoading(true); setPlan(null);
    const takenCourses = takenInput.split(",").map(s => s.trim().toUpperCase()).filter(Boolean);
    try {
      const res  = await fetch(`${API}/api/plan`, {
        method: "POST", headers: { "Content-Type": "application/json", ...(token ? { Authorization: `Bearer ${token}` } : {}) },
        body: JSON.stringify({ major, takenCourses }),
      });
      const data = await res.json();
      if (data.error) { setError(data.error); return; }
      setPlan(parsePlan(data.rawPlan));
      setAiReview(data.aiReview);
      setAiReview(data.aiReview);
    } catch { setError("Could not connect to the server."); }
    finally { setLoading(false); }
  };
  const exportToPDF = () => {
    const printWindow = window.open('', '_blank');
    const majorName = MAJORS.find(m => m.code === major)?.name || major;
    
    const quartersHTML = plan.quarters.map(q => {
        const { season, year } = quarterLabel(q.number);
        return `
            <div class="quarter">
                <div class="quarter-header">
                    <span class="season">Year ${year} — ${season}</span>
                    <span class="units">${q.units} units · avg difficulty ${q.avgDifficulty.toFixed(1)}/10</span>
                </div>
                ${q.courses.map(c => `
                    <div class="course ${c.isBreadth ? 'breadth' : ''}">
                        <div class="course-left">
                            <span class="course-id">${c.id}</span>
                            ${c.juniorPlus ? '<span class="tag">Junior+</span>' : ''}
                            <span class="course-name">${c.name}</span>
                        </div>
                        <span class="course-diff">diff: ${c.difficulty.toFixed(1)}</span>
                    </div>
                `).join('')}
            </div>
        `;
    }).join('');

    const completedHTML = plan.completed.length > 0 ? `
        <div class="completed">
            <div class="section-title">✓ Already Completed (${completedUnits} units)</div>
            <div class="completed-list">
                ${plan.completed.map(c => `<span class="chip">${c.id}</span>`).join('')}
            </div>
        </div>
    ` : '';

    printWindow.document.write(`
        <!DOCTYPE html>
        <html>
        <head>
            <title>UCR Course Plan — ${majorName}</title>
            <style>
                * { box-sizing: border-box; margin: 0; padding: 0; }
                body { font-family: 'Courier New', monospace; background: white; color: #0a0e1a; padding: 32px; font-size: 11px; }
                h1 { font-size: 20px; font-weight: 800; margin-bottom: 4px; }
                .subtitle { font-size: 10px; color: #64748b; letter-spacing: 0.1em; margin-bottom: 24px; }
                .stats { display: flex; gap: 24px; margin-bottom: 24px; padding: 12px 16px; border: 1px solid #e2e8f0; border-radius: 8px; }
                .stat-label { font-size: 8px; color: #94a3b8; letter-spacing: 0.12em; }
                .stat-value { font-size: 18px; font-weight: 700; }
                .completed { margin-bottom: 20px; padding: 12px 16px; border: 1px solid #dcfce7; border-radius: 8px; background: #f0fdf4; }
                .completed-list { display: flex; flex-wrap: wrap; gap: 6px; margin-top: 8px; }
                .chip { background: #dcfce7; border: 1px solid #86efac; border-radius: 4px; padding: 2px 8px; font-size: 10px; }
                .section-title { font-size: 9px; font-weight: 700; letter-spacing: 0.12em; color: #16a34a; }
                .year-header { font-size: 10px; font-weight: 700; letter-spacing: 0.14em; color: #64748b; margin: 16px 0 8px; border-bottom: 1px solid #e2e8f0; padding-bottom: 4px; }
                .quarter { margin-bottom: 12px; border: 1px solid #e2e8f0; border-radius: 8px; overflow: hidden; break-inside: avoid; }
                .quarter-header { display: flex; justify-content: space-between; padding: 8px 12px; background: #f8fafc; border-bottom: 1px solid #e2e8f0; }
                .season { font-weight: 700; font-size: 10px; color: #2563eb; letter-spacing: 0.08em; }
                .units { font-size: 9px; color: #94a3b8; }
                .course { display: flex; justify-content: space-between; align-items: center; padding: 6px 12px; border-bottom: 1px solid #f1f5f9; }
                .course.breadth { background: #eff6ff; }
                .course:last-child { border-bottom: none; }
                .course-left { display: flex; align-items: center; gap: 8px; }
                .course-id { font-weight: 700; font-size: 10px; min-width: 80px; }
                .course-name { color: #64748b; font-size: 10px; }
                .course-diff { font-size: 9px; color: #94a3b8; }
                .tag { font-size: 8px; background: #ede9fe; color: #7c3aed; padding: 1px 5px; border-radius: 3px; }
                @media print { body { padding: 16px; } }
            </style>
        </head>
        <body>
            <h1>UCR Course Plan</h1>
            <div class="subtitle">${majorName.toUpperCase()} · BOURNS COLLEGE OF ENGINEERING · ${new Date().toLocaleDateString()}</div>
            <div class="stats">
                <div><div class="stat-label">COMPLETED</div><div class="stat-value">${completedUnits}u</div></div>
                <div><div class="stat-label">REMAINING</div><div class="stat-value">${remainingUnits}u</div></div>
                <div><div class="stat-label">QUARTERS</div><div class="stat-value">${plan.quarters.length}</div></div>
                <div><div class="stat-label">PROGRESS</div><div class="stat-value">${progressPct}%</div></div>
            </div>
            ${completedHTML}
            ${quartersHTML}
        </body>
        </html>
    `);
    printWindow.document.close();
    setTimeout(() => printWindow.print(), 500);
};

  return (
    <div style={{ minHeight: "100vh", background: "#080c18", color: "#e2e8f0", fontFamily: "'IBM Plex Mono', 'Courier New', monospace" }}>
      {/* Header */}
      <div style={{ borderBottom: "1px solid #0d1525", padding: "16px 32px", display: "flex", alignItems: "center", justifyContent: "space-between", background: "#0a0e1a", position: "sticky", top: 0, zIndex: 50 }}>
        <div style={{ display: "flex", alignItems: "center", gap: "12px" }}>
          <div style={{ width: "30px", height: "30px", borderRadius: "7px", background: "linear-gradient(135deg, #2563eb, #7c3aed)", display: "flex", alignItems: "center", justifyContent: "center", fontSize: "13px", fontWeight: "800", color: "white" }}>R</div>
          <div>
            <div style={{ fontSize: "13px", fontWeight: "600", color: "#f1f5f9" }}>UCR Course Planner</div>
            <div style={{ fontSize: "9px", color: "#1e2d40", letterSpacing: "0.1em" }}>BOURNS COLLEGE OF ENGINEERING</div>
          </div>
        </div>
        <div style={{ display: "flex", alignItems: "center", gap: "16px" }}>
          {plan && <div style={{ fontSize: "10px", color: "#475569" }}><span style={{ color: "#4ade80" }}>{completedUnits}</span> / {TOTAL_UNITS} units</div>}
          <div style={{ fontSize: "11px", color: "#475569" }}>
            <span style={{ color: "#3b82f6" }}>{user.netId}</span>
            {user.fullName && <span style={{ color: "#334155" }}> · {user.fullName}</span>}
          </div>
          {plan && (
    <button onClick={exportToPDF} style={{ background: "none", border: "1px solid #141e30", borderRadius: "6px", color: "#3b82f6", fontSize: "10px", fontFamily: "inherit", padding: "5px 10px", cursor: "pointer", letterSpacing: "0.08em" }}>
        EXPORT PDF
    </button>
)}
<button onClick={onLogout} style={{ background: "none", border: "1px solid #141e30", borderRadius: "6px", color: "#475569", fontSize: "10px", fontFamily: "inherit", padding: "5px 10px", cursor: "pointer", letterSpacing: "0.08em" }}>SIGN OUT</button>
          <button onClick={onLogout} style={{ background: "none", border: "1px solid #141e30", borderRadius: "6px", color: "#475569", fontSize: "10px", fontFamily: "inherit", padding: "5px 10px", cursor: "pointer", letterSpacing: "0.08em" }}>SIGN OUT</button>
        </div>
      </div>

      <div style={{ maxWidth: "1100px", margin: "0 auto", padding: "28px 18px" }}>

        {/* Config */}
        <div style={{ background: "#0a0e1a", border: "1px solid #0d1525", borderRadius: "14px", padding: "24px", marginBottom: "20px" }}>
          <div style={{ fontSize: "9px", color: "#1e2d40", letterSpacing: "0.14em", marginBottom: "16px" }}>PLAN CONFIGURATION</div>
          <div style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: "16px", marginBottom: "16px" }}>
            <div>
              <label style={{ display: "block", fontSize: "9px", color: "#334155", letterSpacing: "0.14em", marginBottom: "6px" }}>MAJOR</label>
              <select value={major} onChange={e => setMajor(e.target.value)} style={{ width: "100%", padding: "8px 11px", background: "#0d1220", border: "1px solid #0d1525", borderRadius: "8px", color: major ? "#f1f5f9" : "#334155", fontSize: "12px", fontFamily: "inherit", outline: "none", cursor: "pointer" }}>
                <option value="">Select a major...</option>
                {MAJORS.map(m => <option key={m.code} value={m.code}>{m.name}</option>)}
              </select>
            </div>
            <div>
              <label style={{ display: "block", fontSize: "9px", color: "#334155", letterSpacing: "0.14em", marginBottom: "6px" }}>COMPLETED COURSES <span style={{ color: "#0d1525" }}>(comma-separated)</span></label>
              <div style={{ display: "flex", gap: "8px" }}>
                <input type="text" value={takenInput} onChange={e => setTakenInput(e.target.value)} placeholder="e.g. CS010A, MATH009A" style={{ flex: 1, padding: "8px 11px", background: "#0d1220", border: "1px solid #0d1525", borderRadius: "8px", color: "#f1f5f9", fontSize: "12px", fontFamily: "inherit", outline: "none" }} />
                <button onClick={saveTakenCourses} disabled={savingTaken} title="Save to your account" style={{ padding: "8px 12px", background: "#0d1220", border: "1px solid #0d1525", borderRadius: "8px", color: savingTaken ? "#334155" : "#4ade80", fontSize: "14px", cursor: "pointer" }}>
                  {savingTaken ? "…" : "💾"}
                </button>
              </div>
            </div>
          </div>
          {error && <div style={{ background: "#1a0808", border: "1px solid #7f1d1d", borderRadius: "8px", padding: "8px 13px", fontSize: "11px", color: "#fca5a5", marginBottom: "12px" }}>{error}</div>}
          <button onClick={handleGenerate} disabled={loading} style={{ padding: "10px 24px", background: loading ? "#0d1525" : "linear-gradient(135deg, #2563eb, #7c3aed)", border: "none", borderRadius: "8px", color: loading ? "#334155" : "white", fontSize: "11px", fontFamily: "inherit", fontWeight: "700", letterSpacing: "0.08em", cursor: loading ? "not-allowed" : "pointer" }}>
            {loading ? "GENERATING..." : "GENERATE PLAN →"}
          </button>
        </div>

        {plan && (
          <>
            {/* Progress */}
            <div style={{ background: "#0a0e1a", border: "1px solid #0d1525", borderRadius: "14px", padding: "20px 24px", marginBottom: "18px" }}>
              <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center", marginBottom: "8px" }}>
                <div style={{ fontSize: "9px", color: "#1e2d40", letterSpacing: "0.14em" }}>DEGREE PROGRESS</div>
                <div style={{ fontSize: "10px", color: "#475569", fontFamily: "monospace" }}><span style={{ color: "#4ade80", fontWeight: "700" }}>{completedUnits}</span> / {TOTAL_UNITS} units</div>
              </div>
              <div style={{ width: "100%", height: "7px", borderRadius: "4px", background: "#0d1220", overflow: "hidden", position: "relative" }}>
                <div style={{ position: "absolute", left: 0, top: 0, height: "100%", width: `${progressPct}%`, background: "linear-gradient(90deg, #16a34a, #4ade80)", borderRadius: "4px" }} />
                <div style={{ position: "absolute", left: `${progressPct}%`, top: 0, height: "100%", width: `${plannedPct}%`, background: "linear-gradient(90deg, #2563eb, #7c3aed)", borderRadius: "0 4px 4px 0" }} />
              </div>
              <div style={{ display: "flex", gap: "14px", marginTop: "7px" }}>
                {[["#4ade80", `Completed (${completedUnits}u)`], ["#3b82f6", `Planned (${remainingUnits}u)`], ["#0d1220", `Remaining (${Math.max(0, TOTAL_UNITS - completedUnits - remainingUnits)}u)`]].map(([c, l]) => (
                  <div key={l} style={{ display: "flex", alignItems: "center", gap: "5px" }}>
                    <div style={{ width: "7px", height: "7px", borderRadius: "2px", background: c, border: c === "#0d1220" ? "1px solid #141e30" : "none" }} />
                    <span style={{ fontSize: "9px", color: "#334155" }}>{l}</span>
                  </div>
                ))}
              </div>
              <div style={{ display: "flex", gap: "22px", marginTop: "12px", paddingTop: "12px", borderTop: "1px solid #0d1525" }}>
                {[["PROGRESS", `${progressPct}%`], ["QUARTERS LEFT", plan.quarters.length], ["COURSES LEFT", plan.quarters.reduce((a, q) => a + q.courses.length, 0)], ["UNITS LEFT", remainingUnits]].map(([l, v]) => (
                  <div key={l}><div style={{ fontSize: "8px", color: "#1e2d40", letterSpacing: "0.12em" }}>{l}</div><div style={{ fontSize: "18px", fontWeight: "700", color: "#f1f5f9", marginTop: "2px", fontFamily: "monospace" }}>{v}</div></div>
                ))}
              </div>
            </div>

            {/* AI Review */}
            <div style={{ background: "#0a0e1a", border: "1px solid #0d1525", borderRadius: "14px", padding: "20px 24px", marginBottom: "18px" }}>
              <div style={{ fontSize: "9px", color: "#1e2d40", letterSpacing: "0.14em", marginBottom: "12px" }}>
                ✦ AI ADVISOR REVIEW
              </div>
              {aiReview ? (
                <div style={{ fontSize: "12px", color: "#94a3b8", lineHeight: "1.7", whiteSpace: "pre-wrap" }}>
                  {aiReview}
                </div>
              ) : (
                <div style={{ fontSize: "11px", color: "#1e2d40", fontStyle: "italic" }}>
                  AI review unavailable — Gemini API credits needed. The plan above was generated by the algorithm.
                </div>
              )}
            </div>

            {/* Search */}
            <div style={{ background: "#0a0e1a", border: "1px solid #0d1525", borderRadius: "12px", padding: "16px 20px", marginBottom: "18px" }}>
              <div style={{ fontSize: "9px", color: "#1e2d40", letterSpacing: "0.14em", marginBottom: "8px" }}>COURSE SEARCH</div>
              <SearchBar courseIndex={courseIndex} onSelect={c => setModal({ course: c, qlabel: c.quarterNumber === 0 ? "Completed" : quarterLabel(c.quarterNumber).label })} />
            </div>

            {/* Completed */}
            {plan.completed.length > 0 && (
              <div style={{ background: "#080f08", border: "1px solid #0d2010", borderRadius: "12px", padding: "14px 18px", marginBottom: "18px" }}>
                <div style={{ fontSize: "9px", color: "#4ade80", letterSpacing: "0.14em", marginBottom: "8px" }}>✓ ALREADY COMPLETED — {completedUnits} UNITS</div>
                <div style={{ display: "flex", flexWrap: "wrap", gap: "5px" }}>
                  {plan.completed.map(c => (
                    <div key={c.id} onClick={() => setModal({ course: c, qlabel: "Completed" })} style={{ background: "#0a1f0a", border: "1px solid #0d2a10", borderRadius: "5px", padding: "3px 9px", fontSize: "10px", color: "#86efac", fontFamily: "monospace", cursor: "pointer" }}
                      onMouseEnter={e => e.currentTarget.style.borderColor = "#4ade80"}
                      onMouseLeave={e => e.currentTarget.style.borderColor = "#0d2a10"}
                    >{c.id}</div>
                  ))}
                </div>
              </div>
            )}

            {/* Quarters by year */}
            {(() => {
              const years = {};
              for (const q of plan.quarters) { const { year } = quarterLabel(q.number); if (!years[year]) years[year] = []; years[year].push(q); }
              return Object.entries(years).map(([year, qs]) => (
                <div key={year} style={{ marginBottom: "24px" }}>
                  <div style={{ display: "flex", alignItems: "center", gap: "10px", marginBottom: "12px" }}>
                    <div style={{ fontSize: "9px", fontWeight: "700", letterSpacing: "0.16em", color: "#1e2d40", padding: "3px 11px", border: "1px solid #0d1525", borderRadius: "20px" }}>YEAR {year}</div>
                    <div style={{ flex: 1, height: "1px", background: "#0d1525" }} />
                  </div>
                  <div style={{ display: "grid", gridTemplateColumns: "repeat(auto-fill, minmax(290px, 1fr))", gap: "10px" }}>
                    {qs.map(q => {
                      const { season } = quarterLabel(q.number);
                      return (
                        <div key={q.number} style={{ background: "#0a0e1a", border: "1px solid #0d1525", borderRadius: "12px", padding: "14px" }}>
                          <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center", marginBottom: "10px" }}>
                            <div>
                              <div style={{ fontSize: "10px", fontWeight: "700", color: "#2563eb", letterSpacing: "0.1em" }}>{season.toUpperCase()}</div>
                              <div style={{ fontSize: "8px", color: "#1e2d40", marginTop: "1px" }}>{q.units} units · diff {q.avgDifficulty.toFixed(1)}</div>
                            </div>
                            <DiffBar score={q.avgDifficulty} width={40} />
                          </div>
                          <div style={{ display: "flex", flexDirection: "column", gap: "5px" }}>
                            {q.courses.map(c => (
                              <div key={c.id} onClick={() => setModal({ course: c, qlabel: quarterLabel(q.number).label })}
                                style={{ background: c.isBreadth ? "#080f1a" : "#0d1220", border: `1px solid ${c.isBreadth ? "#0a1a2e" : "#0d1525"}`, borderRadius: "7px", padding: "7px 10px", display: "flex", justifyContent: "space-between", alignItems: "center", cursor: "pointer", transition: "border-color 0.15s" }}
                                onMouseEnter={e => e.currentTarget.style.borderColor = "#2563eb"}
                                onMouseLeave={e => e.currentTarget.style.borderColor = c.isBreadth ? "#0a1a2e" : "#0d1525"}
                              >
                                <div>
                                  <div style={{ display: "flex", alignItems: "center", gap: "4px" }}>
                                    <span style={{ fontSize: "10px", fontWeight: "700", color: c.isBreadth ? "#3b82f6" : "#e2e8f0", fontFamily: "monospace" }}>{c.id}</span>
                                    {c.juniorPlus && <span style={{ fontSize: "7px", background: "#1e1b4b", color: "#a5b4fc", padding: "1px 4px", borderRadius: "3px" }}>J+</span>}
                                  </div>
                                  <div style={{ fontSize: "9px", color: "#1e2d40", marginTop: "1px" }}>{c.name.length > 30 ? c.name.slice(0, 30) + "…" : c.name}</div>
                                </div>
                                <DiffBar score={c.difficulty} width={32} />
                              </div>
                            ))}
                          </div>
                        </div>
                      );
                    })}
                  </div>
                </div>
              ));
            })()}
          </>
        )}
      </div>

      <CourseModal course={modal?.course} qlabel={modal?.qlabel} courseIndex={courseIndex} prereqMap={prereqMap} onClose={() => setModal(null)} />
    </div>
  );
}

// ── Root App ──────────────────────────────────────────────────────────────────

export default function App() {
  const [user, setUser] = useState(() => {
    try { return JSON.parse(localStorage.getItem("ucr_user")); } catch { return null; }
  });

  const handleLogin  = (data) => setUser({ netId: data.netId, fullName: data.fullName, major: data.major });
  const handleLogout = () => { localStorage.removeItem("ucr_token"); localStorage.removeItem("ucr_user"); setUser(null); };

  if (!user) return <AuthPage onLogin={handleLogin} />;
  return <Planner user={user} onLogout={handleLogout} />;
}