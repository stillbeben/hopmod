
function Bullethole(attachTo){
    var img = new Image();
    img.src = "images/background_decal.png";
    img.onload = function(){
        img.style.position = "absolute";
        img.style.left = (Math.random() * (attachTo.clientWidth - img.width)) + "px";
        img.style.top = (Math.random() * (attachTo.clientHeight - img.height)) + "px";
        img.style.zIndex = -1;
        img.style.opacity = Math.random();
        img.unselectable = "on";
        attachTo.appendChild(img);
    }
}

window.onload = function(){
    var n = Math.random() * 50;
    for(var i = 0; i < n ; i++) new Bullethole(document.body);
}
