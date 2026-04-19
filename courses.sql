-- ============================================================
-- UCR BCOE Required Courses Database
-- Covers: CS, CE, EE, ME, ChemE, BioE, EnvE, MSE, DS, Robotics
-- priority_score and difficulty_score are on a scale of 1.0 - 10.0
-- ============================================================

CREATE TABLE IF NOT EXISTS courses (
    course_id               TEXT PRIMARY KEY,
    course                  TEXT NOT NULL,
    units                   INTEGER NOT NULL,
    prerequisite            TEXT,
    major                   TEXT NOT NULL,
    division                TEXT NOT NULL,        -- 'lower' or 'upper'
    priority_score          REAL NOT NULL DEFAULT 5.0,
    difficulty_score        REAL NOT NULL DEFAULT 5.0,
    junior_plus_standing    INTEGER NOT NULL DEFAULT 0,
    FOREIGN KEY (prerequisite) REFERENCES courses(course_id)
);

-- ============================================================
-- SHARED / COMMON COURSES
-- ============================================================

-- Calculus (all BCOE majors)
INSERT INTO courses VALUES ('MATH009A', 'Calculus I',                          4, NULL,       'ALL',                               'lower', 6.0, 5.5, 0);
INSERT INTO courses VALUES ('MATH009B', 'Calculus II',                         4, 'MATH009A', 'ALL',                               'lower', 6.0, 6.0, 0);
INSERT INTO courses VALUES ('MATH009C', 'Calculus III',                        4, 'MATH009B', 'ALL',                               'lower', 6.0, 6.5, 0);

-- Physics
INSERT INTO courses VALUES ('PHYS040A', 'Physics - Mechanics',                 5, 'MATH009A', 'CS,CE,EE,ME,BIOE,ENVE,MSE,DS,ROBO', 'lower', 7.0, 7.0, 0);
INSERT INTO courses VALUES ('PHYS040B', 'Physics - Heat/Waves/Sound',          5, 'PHYS040A', 'CS,CE,EE,ME,BIOE,ENVE,MSE,ROBO',    'lower', 7.0, 7.0, 0);
INSERT INTO courses VALUES ('PHYS040C', 'Physics - Electricity & Magnetism',   5, 'PHYS040B', 'CS,CE,EE,ME',                       'lower', 7.0, 8.0, 0);

-- Chemistry
INSERT INTO courses VALUES ('CHEM001A',  'General Chemistry I',                4, NULL,       'CHEME,BIOE,ENVE,MSE',               'lower', 6.0, 6.0, 0);
INSERT INTO courses VALUES ('CHEM01LA',  'General Chemistry Lab I',            2, 'CHEM001A', 'CHEME,BIOE,ENVE,MSE',               'lower', 5.0, 4.5, 0);
INSERT INTO courses VALUES ('CHEM001B',  'General Chemistry II',               4, 'CHEM001A', 'CHEME,BIOE,ENVE,MSE',               'lower', 6.0, 6.5, 0);
INSERT INTO courses VALUES ('CHEM01LB',  'General Chemistry Lab II',           2, 'CHEM001B', 'CHEME,BIOE,ENVE,MSE',               'lower', 5.0, 4.5, 0);
INSERT INTO courses VALUES ('CHEM001C',  'General Chemistry III',              4, 'CHEM001B', 'CHEME,ENVE,MSE',                    'lower', 6.0, 7.0, 0);
INSERT INTO courses VALUES ('CHEM01LC',  'General Chemistry Lab III',          2, 'CHEM001C', 'CHEME,ENVE,MSE',                    'lower', 5.0, 4.5, 0);

-- Biology
INSERT INTO courses VALUES ('BIOL005A',  'Cell Biology',                       4, NULL,       'BIOE,MSE',                          'lower', 6.0, 6.0, 0);
INSERT INTO courses VALUES ('BIOL05LA',  'Cell Biology Lab',                   2, 'BIOL005A', 'BIOE,MSE',                          'lower', 5.0, 4.0, 0);

-- Engineering Intro
INSERT INTO courses VALUES ('ENGR001I',  'Intro to Engineering',               1, NULL,       'ALL',                               'lower', 3.0, 1.5, 0);
INSERT INTO courses VALUES ('ENGR101I',  'Junior Engineering Seminar',         1, NULL,       'ALL',                               'upper', 3.0, 1.5, 1);

-- Math support courses
INSERT INTO courses VALUES ('MATH046',  'Ordinary Differential Equations',     4, 'MATH009C', 'CE,EE,ME,CHEME,BIOE,ENVE,MSE,ROBO', 'lower', 6.5, 7.0, 0);
INSERT INTO courses VALUES ('MATH031',  'Applied Linear Algebra',              4, 'MATH009B', 'CS,CE,EE,DS,ROBO',                  'lower', 6.5, 6.5, 0);
INSERT INTO courses VALUES ('MATH011',  'Intro to Discrete Structures',        4, 'MATH009A', 'CS,CE,DS,ROBO',                     'lower', 7.0, 6.0, 0);
INSERT INTO courses VALUES ('MATH138',  'Probability & Statistics for Engr',   4, 'MATH009C', 'CS,CE,EE,ME,DS,ROBO',               'upper', 7.0, 7.0, 0);

-- Technical Writing
INSERT INTO courses VALUES ('ENGR180W', 'Technical Communication for Engr',    4, NULL,       'CS,CE',                             'upper', 5.0, 3.5, 1);

-- ============================================================
-- COMPUTER SCIENCE (CS)
-- ============================================================

INSERT INTO courses VALUES ('CS010A',  'C++ Programming I',                    4, NULL,       'CS,CE',                             'lower', 6.0, 3.0, 0);
INSERT INTO courses VALUES ('CS010B',  'C++ Programming II',                   4, 'CS010A',   'CS,CE',                             'lower', 6.5, 4.0, 0);
INSERT INTO courses VALUES ('CS010C',  'Intro to Data Structures & Algorithms',4, 'CS010B',   'CS,CE,DS',                          'lower', 7.0, 5.0, 0);
INSERT INTO courses VALUES ('CS061',   'Machine Org. & Assembly Lang.',        4, 'CS010B',   'CS,CE',                             'lower', 7.5, 6.0, 0);
INSERT INTO courses VALUES ('CS100',   'Software Construction',                4, 'CS010C',   'CS,CE',                             'upper', 8.0, 6.0, 0);
INSERT INTO courses VALUES ('CS111',   'Discrete Structures',                  4, 'CS010C',   'CS,CE',                             'upper', 8.5, 7.0, 0);
INSERT INTO courses VALUES ('CS120A',  'Logic Design',                         5, 'CS061',    'CS,CE',                             'upper', 8.0, 7.0, 0);
INSERT INTO courses VALUES ('CS120B',  'Embedded Systems',                     4, 'CS120A',   'CS,CE',                             'upper', 8.0, 8.0, 0);
INSERT INTO courses VALUES ('CS141',   'Interm. Data Structures & Algorithms', 4, 'CS111',    'CS,CE,DS',                          'upper', 9.0, 8.0, 0);
INSERT INTO courses VALUES ('CS150',   'Automata & Formal Languages',          4, 'CS111',    'CS',                                'upper', 8.5, 8.5, 0);
INSERT INTO courses VALUES ('CS152',   'Compiler Design',                      4, 'CS150',    'CS',                                'upper', 9.0, 9.5, 1);
INSERT INTO courses VALUES ('CS153',   'Operating Systems',                    4, 'CS141',    'CS,CE',                             'upper', 9.5, 9.0, 1);
INSERT INTO courses VALUES ('CS161',   'Design & Architecture of Comp. Syst.', 4, 'CS120B',   'CS,CE',                             'upper', 8.5, 8.5, 1);
INSERT INTO courses VALUES ('CS178A',  'Project in CS - Part A',               4, 'CS153',    'CS',                                'upper', 9.8, 7.0, 1);
INSERT INTO courses VALUES ('CS178B',  'Project in CS - Part B',               4, 'CS178A',   'CS',                                'upper', 9.8, 7.0, 1);

-- ============================================================
-- COMPUTER ENGINEERING (CE)
-- ============================================================

INSERT INTO courses VALUES ('EE001A',  'Electrical Circuits I',                4, 'PHYS040B', 'CE,EE',                             'lower', 7.0, 7.0, 0);
INSERT INTO courses VALUES ('EE001LA', 'Electrical Circuits Lab I',            2, 'EE001A',   'CE,EE',                             'lower', 6.0, 5.0, 0);
INSERT INTO courses VALUES ('EE002',   'Electrical Circuits II',               4, 'EE001A',   'CE,EE',                             'lower', 7.0, 7.5, 0);
INSERT INTO courses VALUES ('EE110',   'Probability & Random Processes',       4, 'MATH009C', 'CE,EE',                             'upper', 8.0, 8.0, 0);
INSERT INTO courses VALUES ('EE115',   'Digital Signal Processing',            4, 'EE110',    'CE,EE',                             'upper', 8.5, 8.5, 1);
INSERT INTO courses VALUES ('EE134',   'Fundamentals of VLSI Design',          4, 'CS120A',   'CE',                                'upper', 8.5, 9.0, 1);
INSERT INTO courses VALUES ('EE144',   'Intro to Robotics & Autonomous Systs', 4, 'MATH046',  'CE,ROBO',                           'upper', 8.5, 8.5, 1);

-- ============================================================
-- ELECTRICAL ENGINEERING (EE)
-- ============================================================

INSERT INTO courses VALUES ('EE030A',  'Electronics I',                        4, 'EE001A',   'EE',                                'lower', 7.5, 7.5, 0);
INSERT INTO courses VALUES ('EE030LA', 'Electronics Lab I',                    2, 'EE030A',   'EE',                                'lower', 6.0, 5.0, 0);
INSERT INTO courses VALUES ('EE030B',  'Electronics II',                       4, 'EE030A',   'EE',                                'upper', 8.0, 8.0, 0);
INSERT INTO courses VALUES ('EE100A',  'Linear Systems I',                     4, 'MATH046',  'EE',                                'upper', 8.0, 8.0, 0);
INSERT INTO courses VALUES ('EE100B',  'Linear Systems II',                    4, 'EE100A',   'EE',                                'upper', 8.5, 8.5, 1);
INSERT INTO courses VALUES ('EE120A',  'Electromagnetics I',                   4, 'PHYS040C', 'EE',                                'upper', 8.5, 9.0, 0);
INSERT INTO courses VALUES ('EE120B',  'Electromagnetics II',                  4, 'EE120A',   'EE',                                'upper', 8.5, 9.5, 1);
INSERT INTO courses VALUES ('EE175',   'EE Senior Design Project',             4, 'EE100A',   'EE',                                'upper', 9.5, 7.0, 1);

-- ============================================================
-- MECHANICAL ENGINEERING (ME)
-- ============================================================

INSERT INTO courses VALUES ('ME002',   'Intro to Mechanical Engineering',      2, NULL,       'ME',                                'lower', 4.0, 2.5, 0);
INSERT INTO courses VALUES ('ME009',   'Intro to Engineering Graphics',        4, NULL,       'ME',                                'lower', 4.5, 3.5, 0);
INSERT INTO courses VALUES ('ME010',   'Engineering Graphics & CAD',           4, NULL,       'ME,ROBO',                           'lower', 4.5, 3.5, 0);
INSERT INTO courses VALUES ('ME018A',  'Statics',                              4, 'PHYS040A', 'ME',                                'lower', 7.0, 7.0, 0);
INSERT INTO courses VALUES ('ME018B',  'Dynamics',                             4, 'ME018A',   'ME',                                'lower', 7.5, 7.5, 0);
INSERT INTO courses VALUES ('ME100',   'Engineering Thermodynamics',           4, 'MATH009C', 'ME',                                'upper', 8.0, 8.0, 0);
INSERT INTO courses VALUES ('ME103',   'Fluid Mechanics',                      4, 'ME018B',   'ME',                                'upper', 8.5, 8.5, 0);
INSERT INTO courses VALUES ('ME104',   'Heat Transfer',                        4, 'ME103',    'ME',                                'upper', 8.5, 8.5, 1);
INSERT INTO courses VALUES ('ME109',   'Mechanics of Materials',               4, 'ME018B',   'ME',                                'upper', 8.0, 8.0, 0);
INSERT INTO courses VALUES ('ME120',   'Manufacturing Processes',              4, NULL,       'ME',                                'upper', 7.5, 7.0, 1);
INSERT INTO courses VALUES ('ME128',   'Control Systems',                      4, 'MATH046',  'ME,ROBO',                           'upper', 9.0, 9.0, 1);
INSERT INTO courses VALUES ('ME175A',  'ME Senior Design Project A',           4, 'ME103',    'ME',                                'upper', 9.5, 7.0, 1);
INSERT INTO courses VALUES ('ME175B',  'ME Senior Design Project B',           4, 'ME175A',   'ME',                                'upper', 9.5, 7.0, 1);

-- ============================================================
-- CHEMICAL ENGINEERING (ChemE)
-- ============================================================

INSERT INTO courses VALUES ('CHE100',  'Material & Energy Balances',           4, 'CHEM001B', 'CHEME',                             'upper', 8.0, 8.0, 0);
INSERT INTO courses VALUES ('CHE107A', 'Transport Phenomena I',                4, 'CHE100',   'CHEME',                             'upper', 8.5, 9.0, 1);
INSERT INTO courses VALUES ('CHE107B', 'Transport Phenomena II',               4, 'CHE107A',  'CHEME',                             'upper', 8.5, 9.5, 1);
INSERT INTO courses VALUES ('CHE110',  'Chemical Thermodynamics',              4, 'CHE100',   'CHEME',                             'upper', 8.5, 8.5, 1);
INSERT INTO courses VALUES ('CHE120',  'Chemical Reaction Engineering',        4, 'CHE110',   'CHEME',                             'upper', 9.0, 9.5, 1);
INSERT INTO courses VALUES ('CHE130',  'Process Control',                      4, 'CHE107A',  'CHEME',                             'upper', 8.5, 8.5, 1);
INSERT INTO courses VALUES ('CHE140',  'Chemical Process Design I',            4, 'CHE120',   'CHEME',                             'upper', 9.5, 9.0, 1);
INSERT INTO courses VALUES ('CHE141',  'Chemical Process Design II',           4, 'CHE140',   'CHEME',                             'upper', 9.5, 9.0, 1);
INSERT INTO courses VALUES ('CHE150',  'ChemE Lab I',                          2, 'CHE100',   'CHEME',                             'upper', 7.0, 6.0, 0);
INSERT INTO courses VALUES ('CHE151',  'ChemE Lab II',                         2, 'CHE150',   'CHEME',                             'upper', 7.0, 6.0, 1);

-- ============================================================
-- BIOENGINEERING (BioE)
-- ============================================================

INSERT INTO courses VALUES ('BIEN100',  'Intro to Bioengineering',             4, 'BIOL005A', 'BIOE',                              'upper', 7.5, 7.0, 0);
INSERT INTO courses VALUES ('BIEN103',  'Biomechanics',                        4, 'PHYS040A', 'BIOE',                              'upper', 8.0, 8.0, 0);
INSERT INTO courses VALUES ('BIEN110',  'Biomedical Instrumentation',          4, 'EE001A',   'BIOE',                              'upper', 8.5, 8.5, 1);
INSERT INTO courses VALUES ('BIEN115',  'Biotransport',                        4, 'MATH046',  'BIOE',                              'upper', 8.5, 9.0, 1);
INSERT INTO courses VALUES ('BIEN120',  'Biomedical Systems Modeling',         4, 'BIEN100',  'BIOE',                              'upper', 8.5, 8.5, 1);
INSERT INTO courses VALUES ('BIEN150',  'BioE Senior Design Project',          4, 'BIEN120',  'BIOE',                              'upper', 9.5, 7.0, 1);

-- ============================================================
-- ENVIRONMENTAL ENGINEERING (EnvE)
-- ============================================================

INSERT INTO courses VALUES ('ENVE100', 'Environmental Engineering Fund.',      4, 'CHEM001B', 'ENVE',                              'upper', 8.0, 7.5, 0);
INSERT INTO courses VALUES ('ENVE101', 'Water & Wastewater Treatment',         4, 'ENVE100',  'ENVE',                              'upper', 8.5, 8.0, 1);
INSERT INTO courses VALUES ('ENVE110', 'Air Pollution Control',                4, 'ENVE100',  'ENVE',                              'upper', 8.0, 8.0, 1);
INSERT INTO courses VALUES ('ENVE120', 'Solid & Hazardous Waste Management',   4, 'ENVE100',  'ENVE',                              'upper', 8.0, 7.5, 1);
INSERT INTO courses VALUES ('ENVE130', 'Environmental Transport Processes',    4, 'MATH046',  'ENVE',                              'upper', 8.5, 9.0, 1);
INSERT INTO courses VALUES ('ENVE175', 'EnvE Senior Design Project',           4, 'ENVE130',  'ENVE',                              'upper', 9.5, 7.0, 1);

-- ============================================================
-- MATERIALS SCIENCE & ENGINEERING (MSE)
-- ============================================================

INSERT INTO courses VALUES ('MSE101',  'Structure of Materials',               4, 'CHEM001B', 'MSE',                               'upper', 8.0, 7.5, 0);
INSERT INTO courses VALUES ('MSE102',  'Thermodynamics of Materials',          4, 'MSE101',   'MSE',                               'upper', 8.5, 8.5, 1);
INSERT INTO courses VALUES ('MSE103',  'Kinetics of Materials',                4, 'MSE102',   'MSE',                               'upper', 8.5, 9.0, 1);
INSERT INTO courses VALUES ('MSE110',  'Mechanical Behavior of Materials',     4, 'MSE101',   'MSE',                               'upper', 8.5, 8.5, 1);
INSERT INTO courses VALUES ('MSE120',  'Electronic Properties of Materials',   4, 'MSE101',   'MSE',                               'upper', 8.5, 9.0, 1);
INSERT INTO courses VALUES ('MSE175',  'MSE Senior Design Project',            4, 'MSE103',   'MSE',                               'upper', 9.5, 7.0, 1);

-- ============================================================
-- DATA SCIENCE (DS)
-- ============================================================

INSERT INTO courses VALUES ('CS009A',  'Intro to CS for Scientists I',         4, NULL,       'DS',                                'lower', 5.5, 3.0, 0);
INSERT INTO courses VALUES ('CS009B',  'Intro to CS for Scientists II',        4, 'CS009A',   'DS',                                'lower', 6.0, 4.0, 0);
INSERT INTO courses VALUES ('CS009C',  'Intro to CS for Scientists III',       4, 'CS009A',   'DS',                                'lower', 6.0, 4.0, 0);
INSERT INTO courses VALUES ('STAT100A','Intro to Probability',                 4, 'MATH009C', 'DS',                                'upper', 7.5, 7.5, 0);
INSERT INTO courses VALUES ('STAT100B','Intro to Statistics',                  4, 'STAT100A', 'DS',                                'upper', 7.5, 7.5, 0);
INSERT INTO courses VALUES ('CS105',   'Data Analysis Methods',                4, 'CS141',    'DS',                                'upper', 8.5, 7.5, 1);
INSERT INTO courses VALUES ('CS171',   'Intro to Machine Learning',            4, 'CS141',    'DS',                                'upper', 9.0, 9.0, 1);
INSERT INTO courses VALUES ('CS175',   'Data Science Project',                 4, 'CS171',    'DS',                                'upper', 9.5, 7.0, 1);

-- ============================================================
-- ROBOTICS ENGINEERING (ROBO)
-- ============================================================

INSERT INTO courses VALUES ('ROBO101', 'Intro to Robotics',                    4, 'CS010C',   'ROBO',                              'upper', 8.0, 7.5, 0);
INSERT INTO courses VALUES ('ROBO102', 'Robot Motion Planning',                4, 'ROBO101',  'ROBO',                              'upper', 8.5, 8.5, 1);
INSERT INTO courses VALUES ('ROBO120', 'Sensors & Actuators',                  4, 'EE001A',   'ROBO',                              'upper', 8.5, 8.0, 1);
INSERT INTO courses VALUES ('ROBO150', 'Robotics Senior Design Project',       4, 'ROBO102',  'ROBO',                              'upper', 9.5, 7.0, 1);

-- ============================================================
-- BREADTH & TECHNICAL ELECTIVES (all majors)
-- ============================================================

INSERT INTO courses VALUES ('BREADTH1', 'General Breadth Requirement', 4, NULL, 'ALL', 'lower', 3.0, 2.0, 0);
INSERT INTO courses VALUES ('BREADTH2', 'General Breadth Requirement', 4, NULL, 'ALL', 'lower', 3.0, 2.0, 0);
INSERT INTO courses VALUES ('BREADTH3', 'General Breadth Requirement', 4, NULL, 'ALL', 'lower', 3.0, 2.0, 0);
INSERT INTO courses VALUES ('BREADTH4', 'General Breadth Requirement', 4, NULL, 'ALL', 'lower', 3.0, 2.0, 0);
INSERT INTO courses VALUES ('BREADTH5', 'General Breadth Requirement', 4, NULL, 'ALL', 'lower', 3.0, 2.0, 0);
INSERT INTO courses VALUES ('BREADTH6', 'General Breadth Requirement', 4, NULL, 'ALL', 'lower', 3.0, 2.0, 0);
INSERT INTO courses VALUES ('BREADTH7', 'General Breadth Requirement', 4, NULL, 'ALL', 'upper', 3.0, 2.0, 0);
INSERT INTO courses VALUES ('BREADTH8', 'General Breadth Requirement', 4, NULL, 'ALL', 'upper', 3.0, 2.0, 0);



-- ============================================================
-- STUDENT TAKEN COURSES (separate from course catalog)
-- ============================================================
CREATE TABLE IF NOT EXISTS student_courses (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    student_id  TEXT NOT NULL,
    course_id   TEXT NOT NULL,
    taken       INTEGER NOT NULL DEFAULT 1,
    FOREIGN KEY (course_id) REFERENCES courses(course_id),
    UNIQUE(student_id, course_id)
);