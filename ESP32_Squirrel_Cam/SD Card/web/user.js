// Function to display user data in the table
function displayUserData(userData) {
	document.getElementById("chatId").value = "";
    const table = document.getElementById("userTable");
    table.innerHTML = "<tr><th>Chat ID</th><th>Name</th><th>Type</th><th>Action</th></tr>";	
    const users = userData.split(";");
    users.forEach(user => {
		if(!user.includes("|"))
			return;
        const [chatId, type, name] = user.split("|");
        const typeText = ["Empty", "Admin", "Subscriber", "Undefined"][type];
        table.innerHTML += `<tr><td>${chatId}</td><td>${name}</td><td>${typeText}</td><td><button onclick="modifyUser(${chatId}, ${type}, '${name}')">Modify</button> <button onclick="removeUser(${chatId})">Remove</button></td></tr>`;
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
	const userName = document.getElementById("userName").value;
	updateUser(chatId, userType, userName);
}

// Function to remove a user
function removeUser(chatId) {
	updateUser(chatId, 0, "");    
}

function updateUser(chatId, userType, userName){
	var oRequest = new XMLHttpRequest();
	var sURL  = '/user';
	if(chatId > 0){
		sURL  += '?chatId=' + chatId;
		sURL  += '&userType=' + userType;
		sURL  += '&userName=' + userName;
	}
	
	oRequest.open("GET",sURL,true);
	oRequest.onload = function (e) {
		if(oRequest.readyState === 4 && oRequest.status === 200){			
			displayUserData(oRequest.responseText);			
		}
	};
	oRequest.onerror = function (e) {
		console.log("update User failed!")
	};
	oRequest.send(null);
}

// Function to modify a user
function modifyUser(chatId, type, name) {
    document.getElementById("chatId").value = chatId;
    document.getElementById("userType").value = type;
	document.getElementById("userName").value = name;
}
function errorFetchData(error) {
	console.log(error);
	displayUserData("83764|0|Thomas;47111|2|Test;47635|1|Hallo Welt");
}