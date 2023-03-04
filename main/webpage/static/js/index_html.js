const dropZone = document.getElementById("drop-zone");

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
    console.log(file);
    // Do something with the file, such as upload it to a server
});

function updateFirmware() 
{
    // Form Data
    console.log("Update Firmware Called");
    var formData = new FormData();
    var fileSelect = document.getElementById("dropzone-input");
    
    if (fileSelect.files && fileSelect.files.length == 1) 
	{
        var file = fileSelect.files[0];
        formData.set("file", file, file.name);
        // document.getElementById("ota_update_status").innerHTML = "Uploading " + file.name + ", Firmware Update in Progress...";

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

const firmware_update_button = document.getElementById('firmware_update');
firmware_update_button.addEventListener('click', updateFirmware);

