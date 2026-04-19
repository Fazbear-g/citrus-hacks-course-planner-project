// Course data for UCR BCOE
const majors = {
    cs: {
        name: "Computer Science",
        courses: [
            { code: "CS 10", name: "Intro to Programming", priority: 10, difficulty: 3, prereqs: [] },
            { code: "CS 11", name: "Advanced Programming", priority: 10, difficulty: 4, prereqs: ["CS 10"] },
            { code: "CS 12", name: "Data Structures", priority: 10, difficulty: 5, prereqs: ["CS 11"] },
            { code: "CS 14", name: "Intro to Algorithms", priority: 9, difficulty: 6, prereqs: ["CS 12"] },
            // Add more
        ]
    },
    ee: {
        name: "Electrical Engineering",
        courses: [
            { code: "EE 1", name: "Intro to EE", priority: 10, difficulty: 4, prereqs: [] },
            // Add more
        ]
    },
    // Add more majors
};

// GE courses
const geCourses = [
    { code: "MATH 9A", name: "Calculus I", priority: 5, difficulty: 4, prereqs: [] },
    { code: "MATH 9B", name: "Calculus II", priority: 5, difficulty: 5, prereqs: ["MATH 9A"] },
    { code: "PHYS 40A", name: "Physics I", priority: 5, difficulty: 4, prereqs: [] },
    // Add more GE
];

// AP/CC mappings
const apMappings = {
    "Calculus AB": "MATH 9A",
    "Calculus BC": "MATH 9B",
    "Physics C": "PHYS 40A",
    // Add more
};

const ccMappings = {
    "MATH 1A": "MATH 9A",
    "CS 10": "CS 10",
    // Add more
};