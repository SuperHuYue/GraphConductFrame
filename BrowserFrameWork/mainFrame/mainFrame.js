const { ipcRenderer } = require('electron');
const websocket = require('ws')

var wsClient = {};
var arr = new Array(1);//该变量用于展示目前所拥有的所有分层的对象 
const IMGCONTAINERNAME = 'ImgContainer1';

ipcRenderer.on("wsWin:ImgData", (e, Img) => {
    Shower(Img);
});

function wsCloseAll() {
    if (wsClient != null) {
        for (let key in wsClient) {
            if (wsClient[key].wsEntity != null) {
                wsClient[key].wsEntity.close();
                wsClient[key].wsEntity = null;
            }
        }
    }
    // wsClient = null;
}

//移除该ID，会使得在切分界面时候不会进行重连模块
function wsRemoveClientID(id) {
    if (wsClient != null) {
        for (let key in wsClient) {
            if (wsClient[key].wsEntity != null && wsClient[ley] == id) {
                wsClient[key].wsEntity.close();
                wsClient[key].wsEntity = null;
                delete wsClient.key;
            }
        }
    }
}

function wsConnect(tar, id, ip, port) {
    console.log(ip + ", " + port);
    if (tar == null) {
        tar = {};
    }
    if (tar.id == null) {
        tar.id = id;
        tar.id.wsEntity = new WSObj(id, ip, port, "?Alias");
    }
    else {
        if (tar.id != null) {
            if (tar.id.wsEntity != null) {
                tar.id.wsEntity.close();
                tar.id.wsEntity = null;
            }
            delete tar.id;
        }
        tar.id.wsEntity = new WSObj(id, ip, port, "?Alias");
    }
}

ipcRenderer.on('divShowWin:row', (e, row) => {
    wsCloseAll();
    var imgContainer = document.getElementById(IMGCONTAINERNAME);
    imgContainer.innerHTML = "";
    let show_row = row;
    arr = new Array(show_row);
    ////////////////////////////////////////
    let sin_height = parseInt(100 / row);
    console.log("hello: " + sin_height);
    for (let i = 0; i < show_row; ++i) {
        var div = document.createElement('div');
        var img = document.createElement('img');
        img.classList.add('ShowContainer')
        img.id = 'row:' + i.toString() + 'col:0';
        div.id = 'divRow:' + i.toString();
        img.style.width = '100%';
        img.src = "../WebSocketParaSetting/img/init.jpg";
        img.style.height = '100%';
        img.onclick = (e) => {
            ipcRenderer.send('mainFrameImg:click', e.target.id);
        }
        if (i == show_row - 1) {
            // img.style.height = (100 - (sin_height * i)).toString() + '%';
            div.style.height = (100 - (sin_height * i)).toString() + '%';
        }
        else {
            div.style.height = sin_height.toString() + '%';
        }
        div.style.width = '100%';
        div.appendChild(img)
        imgContainer.appendChild(div);
    }
    ////////////////////////////////////////
    //检查窗口移除不存在对象的id
    let now_divs = imgContainer.children;
    var now_Valid_client = {};
    for (let sin in now_divs) {
        let now_imgs = sin.children;
        for (let sin_img in now_imgs) {
            if (wsClient[sin_img.id] != null) {
                //reconnect
                wsConnect(now_Valid_client, sin_img.id, wsClient.sin_img.wsEntity.ip, wsClient.sin_img.wsEntity.port);
            }
        }
    }
    wsClient = null;
    wsClient = now_Valid_client;

});
//在拆分目标的同事对现存对象进行重连
ipcRenderer.on('divShowWin:idxCol', (e, idxcol) => {
    wsCloseAll();
    arr[idxcol[0]] = idxcol[1];
    ////////////////////////////////////////
    var div = document.getElementById('divRow:' + idxcol[0].toString());
    div.innerHTML = "";
    let sin_width = parseInt(100 / idxcol[1]);
    for (let i = 0; i < idxcol[1]; ++i) {
        var img = document.createElement('img');
        img.id = 'row:' + idxcol[0].toString() + 'col:' + i.toString();
        img.src = "../WebSocketParaSetting/img/init.jpg";
        img.style.height = '100%';
        img.onclick = (e) => {
            ipcRenderer.send('mainFrameImg:click', e.target.id);
        }
        if (i == idxcol[1] - 1) {
            img.style.width = (100 - (sin_width * i)).toString() + '%';
            // div.style.height = (100 - (sin_height * i)).toString() + '%';
        }
        else {
            img.style.width = sin_width.toString() + '%';
        }
        div.appendChild(img)
    }
    ////////////////////////////////////////
    //检查窗口移除不存在对象的id
    var imgContainer = document.getElementById(IMGCONTAINERNAME);
    let now_divs = imgContainer.children;
    var now_Valid_client = null;
    for (let sin in now_divs) {
        let now_imgs = sin.children;
        for (let sin_img in now_imgs) {
            if (wsClient[sin_img.id] != null) {
                //reconnect
                wsConnect(now_Valid_client, sin_img.id, wsClient.sin_img.wsEntity.ip, wsClient.sin_img.wsEntity.port);
            }
        }
    }
    wsClient = null;
    wsClient = now_Valid_client;
});


ipcRenderer.on('wsWin:Connect', (e, data) => {
    console.log(data);
    var id = data.wsWinIdentify;
    var ip = data.wsIP;
    var port = data.wsPort;
    wsConnect(wsClient, id, ip, port);
});





class WSObj {
    constructor(id, ip, port, other) {
        this.Uri = "ws://" + ip.toString() + ":" + port.toString() + "/" + other.toString() + "=" + id.toString();
        this.ip = ip;
        this.port = port;
        this.other = other;
        this.wsEntity = null;
        this.reader = null;
        this.id = id;
        this.init();
    }
    init() {
        this.wsEntity = new websocket(this.Uri);
        this.wsEntity.onopen = () => {
            console.log('open..')
        };
        this.wsEntity.onclose = (e) => {
            console.log('close: reason' + e.reason)
        };
        this.wsEntity.onmessage = (evt) => {
            var oriStr = evt.data;
            if (typeof (evt.data) != "string") {
                // var buffer_header = new Uint8Array(evt.data, 0, 9);
                //  console.log(evt.data)
                // // console.log("buffer_header: " + buffer_header);
                // // console.log(buffer_header[0] + " " + buffer_header[1] + " " + buffer_header[2] + " " + buffer_header[3])
                // var header_size = buffer_header[0] | buffer_header[1] << 8 | (buffer_header[2]) << 16 | (buffer_header[3]) << 32;
                // // var header_size = buffer_header[0];
                // var to_size = header_size / 4 - 9;
                var buffer_header = new Uint8Array(evt.data, 0, 13)
                var buffer = [];//9header, 1-row,1-col,1-depth,1-piclen
                for (let i = 0; i < 4 * (9 + 4);) {
                    var tmp = buffer_header[i] | buffer_header[i + 1] << 8 | (buffer_header[i + 2]) << 16 | (buffer_header[i + 3]) << 32;
                    buffer.push(tmp);
                    i += 4;
                }
                console.log(buffer);
                var row = buffer[9];
                var col = buffer[10];
                var deepth = buffer[11];
                var pic_length = buffer[12];
                var pic_data = new Uint8Array(evt.data.slice(13 * 4, pic_length + 13 * 4));
                this.Shower(pic_data, this.id);
                //Customer made data demo is only show pic data
                // if (this.reader == null) {
                //     this.reader = new FileReader();
                //     this.reader.addEventListener("loadend", function (e) {
                //         console.log(e);
                //         var buffer_header = new Uint32Array(e.target.result, 0, 10);
                //         var header_size = parseInt(buffer_header[0]);
                //         var to_size = header_size / 4 - 10;
                //         var buffer = new Uint32Array(e.target.result, 0, 10 + to_size + 4);
                //         var from_id = parseInt(buffer[2]);
                //         var to_list = [];
                //         for (var i = 0; i < to_size; ++i) {
                //             to_list[i] = buffer[i + 10];
                //         }
                //         var row = buffer[10 + to_size];
                //         var col = buffer[11 + to_size];
                //         var deepth = buffer[12 + to_size];
                //         var pic_length = parseInt(buffer[13 + to_size]);
                //         var pic_data = new Uint8Array(e.target.result, (14 + to_size) * 4, pic_length);
                //     });
                // }
                // this.reader.readAsArrayBuffer(new Blob([evt.data], { type: "image/jpeg" }));

            }
            else {
                console.log(evt.data)
                var str = evt.data.substr(0, 4);
                if (str == "?Sys") {
                    if (oriStr.search("/?SysConnectInit=Got") != -1) {//start by ?Sys called systype
                        //writeToScreen('<span style="color: blue;">RESPONSE: ' + evt.data + '</span>');
                        if (this.wsEntity == undefined) {
                            console.log("I do not know why");
                        } else {
                            // ?SysSetMethod = xxx & FromAlias=xxx & ToAlias=xx, xx, xx
                            this.wsEntity.send("?SysSetMethod=ImgRetransmission");//这里简略的认为绝对成功，实际上是有问题的
                        }
                    }
                }
                //往后还可以添加其他参数的设定信息
                //end
            }

        }

        this.wsEntity.onerror = (evt) => {
            console.log(evt.data);
            this.wsEntity.close();
        };
        console.log("init complete..");
    };

    Shower(Img, id) {
        var mF = document.getElementById(id);
        // console.log(" shower:", mF);
        if (mF == null) {
            if (this.wsEntity != null) {
                this.wsEntity.close();
                this.wsEntity = null;
            }
        }
        var blob = new Blob([Img], { type: "image/jpeg" });
        var url = URL.createObjectURL(blob);//这个对象使用之后需要手动进行删除，告诉浏览器该内存无需保存
        URL.revokeObjectURL(mF.src);
        mF.src = null;
        mF.src = url;
        // mF.src = ImgUrl.url.blob;
    }

};





