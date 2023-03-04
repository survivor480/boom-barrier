const dropZone = document.getElementById("drop-zone");

var open_button = document.getElementById('open');

var open_function = (event) => {

    console.log("Open Function called");
    fetch("/open", {
        method: 'POST',
        headers: {
            "Content-type": "application/json; charset=UTF-8"
        }
    });

    window.location = "/";
};

open_button.addEventListener('click', open_function);

var close_button = document.getElementById('close');

var close_function = (event) => {
    console.log("Close Function called");
    fetch("/close", {
        method: 'POST',
        headers: {
            "Content-type": "application/json; charset=UTF-8"
        }
    });

    window.location = "/";
}

close_button.addEventListener('click', close_function);


var barrier_button = document.getElementById('barrier_button');

var barrier_timing_submit_button = (event) => {
    console.log("Barrier Timing Function called");
    const fname = document.getElementById("fname");
    fetch("/barrier_timing", {
        method: 'POST',
        headers: {
            "Content-type": "application/json; charset=UTF-8"
        },
        body: "{\"barrier_timing\": \""+ fname.value +"\"}",
    })

    window.location = "/";
}


barrier_button.addEventListener('click', barrier_timing_submit_button);


