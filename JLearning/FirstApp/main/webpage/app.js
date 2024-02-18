let miCheckbox = document.getElementById('show_temp');
let msg = document.getElementById('msg');
let indicador = document.getElementById('circulo');
let updateTemperature;


miCheckbox.addEventListener('click', function () {
    if (miCheckbox.checked) {
        updateTemp();
        updateTemperature = setInterval(updateTemp, 5000);
    } else {
        clearInterval(updateTemperature);
        msg.innerText = '';
        indicador.style.background="gray";
    }

});

function updateTemp() {
    let request = new XMLHttpRequest();

    request.addEventListener("readystatechange", () => {
        console.log(request, request.readyState);
        if (request.readyState === 4) {
            let value = request.responseText;
            
            if(value>=30){
                indicador.style.background="red";
            }else{
                indicador.style.background="green";
            }
            
            msg.innerText = value;
        }
    });
    request.open('GET', "/showTemp");
    request.responseType = "text";
    request.send();
}a