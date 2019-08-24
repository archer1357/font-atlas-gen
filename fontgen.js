function arrayBufferToBase64( buffer ) {
    var binary = '';
    var bytes = new Uint8Array( buffer );
    var len = bytes.byteLength;
    for (var i = 0; i < len; i++) {
        binary += String.fromCharCode( bytes[ i ] );
    }
    return window.btoa( binary );
}

function loadFont(data) {
    var pointSize=parseInt(document.getElementById('pointSize').value);
    var outputInfo=document.getElementById('outputInfo');
    var outputAtlas=document.getElementById('outputAtlas');
    
    var bytes = new Uint8Array( data );
    
    
    Module['print'] = console.log;
    Module['printErr'] = console.log;

    
    
    var myFont=new Module.MyFont();
    
    myFont.initFontInfo(bytes);
    myFont.calcAttribs(pointSize);
    myFont.initAsciiGlyphs();
    myFont.packGlyphs();
    myFont.bitmapGlyphs();
    
    console.log("rowAdvance:"+myFont.rowAdvance);
    
    var glyphs=myFont.glyphs;
    
    console.log("bits:"+bits.size());
    
    var out={};
    out.texWidth=myFont.texWidth;
    out.texHeight=myFont.texHeight;
    out.rowAdvance=myFont.rowAdvance;
    out.glyphs=new Array(glyphs.size());
    
    for(var i=0;i<glyphs.size();i++) {
        
        var g=glyphs.get(i);
        
        out.glyphs[i]={
            colAdvance: g.colAdvance,
            x: g.x,
            y: g.y,
            width: g.width,
            height: g.height,
            index: g.index,
            offsetX: g.offsetX,
            offsetY: g.offsetY,
        };
    }
    
    
    outputInfo.innerHTML=JSON.stringify(out);
    
    
    //draw bitmap
    outputAtlas.width=myFont.texWidth;
    outputAtlas.height=myFont.texHeight;
    
    var bits=myFont.bits;
    var ctx = outputAtlas.getContext("2d");
    
    var imageData = ctx.getImageData(0, 0, outputAtlas.width, outputAtlas.height);
    var data = imageData.data;
    
    
    for(var i=0;i<outputAtlas.width*outputAtlas.height;i++) {

        for(var j=0;j<3;j++) {
            data[i*4+j]=bits.get(i);
        }
        data[i*4+3]=255;
    }

    ctx.putImageData(imageData, 0, 0);
    
    //cleanup
    myFont.delete();
}

function onFontFileSelect(fn) {
    var reader=new FileReader();
    
    reader.onload=((e)=>{
        loadFont(e.target.result);
    });

    reader.readAsArrayBuffer(fn);
}

window.onload=()=>{
    var fontFileDrop=document.getElementById('fontFileDrop');
    var fontFileChoose=document.getElementById('fontFileChoose');
    
    fontFileChoose.addEventListener('change',(e)=>{
        onFontFileSelect(fontFileChoose.files[0]);
    },false);
    
    fontFileDrop.addEventListener('drop',(e)=>{
        e.stopPropagation();
        e.preventDefault();
        onFontFileSelect(e.dataTransfer.files[0]);
    },false);
    
    fontFileDrop.addEventListener('dragover',(e)=>{
        e.stopPropagation();
        e.preventDefault();
        e.dataTransfer.dropEffect='link';
    },false);
};
