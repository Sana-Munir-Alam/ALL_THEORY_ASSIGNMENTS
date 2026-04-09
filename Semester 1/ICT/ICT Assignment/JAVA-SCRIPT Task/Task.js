const prompt = require('prompt-sync')();
var ToDoList = []

function addTask(Item){
    ToDoList.push(Item);
    console.log("Item Added in the List");
}
function removeTask(Index){
    let Length = ToDoList.length;
    if(Index >= 0 && Index < Length){
        ToDoList.splice(Index,1);
        console.log("Item Removed from the List");
    }else {
        console.log("Error! Invalid index.");
    }
}
function displayTask(){
    let Length = ToDoList.length;
    if(Length == 0){
        console.log("No tasks to display!")
    }else{
        for(let i = 0; i < Length; i++){
            console.log("List: " + ToDoList[i] + " Index: " + i);
        }
    }
    
}
function clearList(){
    ToDoList = [];
    console.log("The to-do list has been cleared!");
}

while (true) {
    const command = prompt("Enter command (add, remove, view, clear, exit): ");
    if (command === "add") {
        const item = prompt("Enter a new task: ");
        addTask(item);

    } else if (command === "remove") {
        const index = parseInt(prompt("Enter the index of the task to remove: "), 10);
        removeTask(index);

    } else if (command === "view") {
        displayTask();

    } else if (command === "clear") {
        clearList();

    } else if (command === "exit") {
        console.log("Exiting the program.");
        break;

    } else {
        console.log("Invalid command. Please try again.");
    }
}
