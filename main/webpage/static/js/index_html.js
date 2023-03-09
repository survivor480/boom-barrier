const dropZone = document.getElementById("drop-zone");
const filePreview = document.getElementById("file-preview");	
const imageValue = document.getElementById("image");
const fileInput = document.getElementById("dropzone-input");
const upload_button = document.getElementById("upload-button");
// Highlight drop zone when dragging over
dropZone.addEventListener("dragover", (event) => {
    event.preventDefault();
    dropZone.classList.add("highlight");
});

// Remove highlight when dragging ends
dropZone.addEventListener("dragleave", (event) => {
    dropZone.classList.remove("highlight");
});

// Handle file drop
dropZone.addEventListener("drop", (event) => {
    event.preventDefault();
    dropZone.classList.remove("highlight");
    const file = event.dataTransfer.files[0];
    fileInput.setAttribute("value", file);
    fileInput.files = event.dataTransfer.files;
    // console.log(file);
    const errorShow = document.getElementById("error-show");
    // Display file name and icon
    const fileExtension = file.name.split(".").pop().toLowerCase();
    if(fileExtension==="bin"){
        errorShow.innerText = "";
        imageValue.setAttribute("src", "bin-file.png");
        const fileNameShow = document.getElementById("fileName");
        fileNameShow.style.color = "#000000";
        fileNameShow.innerHTML = file.name;
    } else {
        errorShow.innerText = "Incompatible file type";
    }
});

fileInput.addEventListener('change', (event) => {
    event.preventDefault();
    dropZone.classList.remove("highlight");
    // console.log(file);
    let file = fileInput.files[0];
    const errorShow = document.getElementById("error-show");
    // Display file name and icon
    const fileExtension = file.name.split(".").pop().toLowerCase();
    if(fileExtension==="bin"){
        errorShow.innerText = "";
        imageValue.setAttribute("src", "bin-file.png");
        const fileNameShow = document.getElementById("fileName");
        fileNameShow.style.color = "#000000";
        fileNameShow.innerHTML = file.name;
    } else {
        errorShow.innerText = "Incompatible file type";
    }
});

const updateFirmware = () => {
    // Form Data
    console.log("Update Firmware Called");
    var formData = new FormData();
    var fileSelect = document.getElementById("dropzone-input");
    
    if (fileSelect.files && fileSelect.files.length == 1) 
	{
        var file = fileSelect.files[0];
        formData.set("file", file, file.name);
        // document.getElementById("ota_update_status").innerHTML = "Uploading " + file.name + ", Firmware Update in Progress...";

        var ota_update = document.getElementById("ota_update");

        ota_update.innerHTML = "Firmware Updating. It takes approx 2 minutes."

        // Http Request
        var request = new XMLHttpRequest();

        // request.upload.addEventListener("progress", updateProgress);
        request.open('POST', "/OTAupdate");
        request.responseType = "blob";
        request.send(formData);
    } 
	else 
	{
        window.alert('Select A File First')
    }
};

const firmware_update_button = document.getElementById('upload-button');
firmware_update_button.addEventListener('click', updateFirmware);

