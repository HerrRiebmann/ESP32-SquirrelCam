// Function to display user data in the table
function displayUserData(userData) {
	document.getElementById("chatId").value = "";
    const table = document.getElementById("userTable");
    table.innerHTML = "<tr><th>Chat ID</th><th>Type</th><th>Action</th></tr>";	
    const users = userData.split(";");
    users.forEach(user => {
		if(!user.includes("|"))
			return;
        const [chatId, type] = user.split("|");
        const typeText = ["Empty", "Admin", "Subscriber", "Undefined"][type];
        table.innerHTML += `<tr><td>${chatId}</td><td>${typeText}</td><td><button onclick="modifyUser(${chatId}, ${type})">Modify</button> <button onclick="removeUser(${chatId})">Remove</button></td></tr>`;
    });
}

// Function to fetch and display user data from the web service
function fetchUserData() {
    fetch("/user")
        .then(response => response.text())
        .then(data => displayUserData(data))
        .catch(error => errorFetchData(error));
}

// Function to add/modify a user
function addUser() {
    const chatId = document.getElementById("chatId").value;
    const userType = document.getElementById("userType").value;

    fetch(`/user?chatId=${chatId}&userType=${userType}`)
		.then(response => response.text())
        .then(data => displayUserData(data))
        .catch(error => console.log(error));
}

// Function to remove a user
function removeUser(chatId) {
    fetch(`/user?chatId=${chatId}&userType=0`)
		.then(response => response.text())
        .then(data => displayUserData(data))
        .catch(error => console.log(error));
}

// Function to modify a user
function modifyUser(chatId, type) {
    document.getElementById("chatId").value = chatId;
    document.getElementById("userType").value = type;	
}
function errorFetchData(error) {
	console.log(error);
	displayUserData("83764|0;47111|2;47635|1");
}