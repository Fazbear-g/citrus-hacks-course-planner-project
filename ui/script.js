// Include data.js
// Assuming data.js is loaded

document.getElementById('inputForm').addEventListener('submit', function(e) {
    e.preventDefault();
    
    const major = document.getElementById('major').value;
    const apCourses = document.getElementById('apCourses').value.split(',').map(s => s.trim());
    const ccCourses = document.getElementById('ccCourses').value.split(',').map(s => s.trim());
    const coursesTaken = document.getElementById('coursesTaken').value.split(',').map(s => s.trim());
    const standing = document.getElementById('standing').value;
    
    // Map AP and CC to UCR courses
    const mappedAP = apCourses.map(course => apMappings[course] || course).filter(c => c);
    const mappedCC = ccCourses.map(course => ccMappings[course] || course).filter(c => c);
    const allTaken = [...coursesTaken, ...mappedAP, ...mappedCC];
    
    // Get major courses
    const majorCourses = majors[major].courses;
    const allCourses = [...majorCourses, ...geCourses];
    
    // Find available courses (prereqs met)
    const availableCourses = allCourses.filter(course => 
        !allTaken.includes(course.code) && 
        course.prereqs.every(prereq => allTaken.includes(prereq))
    );
    
    // Sort by priority desc, then difficulty asc
    availableCourses.sort((a, b) => b.priority - a.priority || a.difficulty - b.difficulty);
    
    // Generate plan
    let planHTML = `<h2>Recommended Course Plan for ${majors[major].name}</h2>`;
    planHTML += '<ul>';
    availableCourses.slice(0, 10).forEach(course => {
        planHTML += `<li>${course.code}: ${course.name} (Priority: ${course.priority}, Difficulty: ${course.difficulty})</li>`;
    });
    planHTML += '</ul>';
    
    document.getElementById('plan').innerHTML = planHTML;
});