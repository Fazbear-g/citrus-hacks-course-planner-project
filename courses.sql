-- courses.sql
CREATE TABLE IF NOT EXISTS courses (
    course_id       TEXT PRIMARY KEY,
    course          TEXT NOT NULL,
    priority_score  REAL NOT NULL,
    difficulty_score REAL NOT NULL,
    junior_plus_standing INTEGER NOT NULL DEFAULT 0  -- 0 = false, 1 = true
);

-- Sample data
INSERT INTO courses VALUES ('CS101', 'Intro to CS',        3.5, 2.0, 0);
INSERT INTO courses VALUES ('CS301', 'Algorithms',         4.8, 4.5, 1);
INSERT INTO courses VALUES ('CS401', 'Operating Systems',  4.2, 4.8, 1);
INSERT INTO courses VALUES ('MATH201', 'Discrete Math',    3.9, 3.7, 0);