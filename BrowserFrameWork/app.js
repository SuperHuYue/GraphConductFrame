//const { app } = require('electron');
const { app, BrowserWindow, Menu, ipcMain } = require('electron')
const path = require('path');
const url = require('url')
// var screen = electron.screen;
// const TreeViewInit = require('./TreeView/treelist');
var WebSocketWin = {};
var mainWindow = null;
var divShowWin = null;

if (process.mas) app.setName("william's graph conductor");
app.on('ready', function () {
    mainWindow = new BrowserWindow({
        width: 1000,
        height: 700,
        // maxWidth: 1500,
        minHeight: 300,
        title: app.getName(),
        show: false,
        webPreferences: { enableRemoteModule: true, contextIsolation: false, nodeIntegration: true },//必加
        useContentSize: false,
    })
    mainWindow.loadURL(url.pathToFileURL(path.join(__dirname, "mainFrame", "mainFrame.html")).href);
    mainWindow.once("ready-to-show", () => {
        mainWindow.show();
    })
    mainWindow.on('resize', () => {
        // var winSize = mainWindow.getSize();
        // mainWindow.webContents.send('Win:resize', winSize, screen.getPrimaryDisplay().scaleFactor);
    })
    const mainMenu = Menu.buildFromTemplate(menutemplate);
    Menu.setApplicationMenu(mainMenu);
})

var menutemplate = [
    {
        label: 'Connect Param',
        submenu: [
            {
                label: 'ShowerDivide',
                click() {
                    divShowWin = new BrowserWindow({
                        width: 500,
                        height: 250,
                        title: "divShowWin",
                        resizable: process.env.NODE_ENV == "production" ? false : true,
                        webPreferences: { nodeIntegration: true, contextIsolation: false, enableRemoteModule: true },
                    });
                    divShowWin.loadURL(url.pathToFileURL(path.join(__dirname, "mainFrameShowDiv", "winMainFrameShowDiv.html")).href);
                    //garbage collection
                    divShowWin.on('close', (e) => {
                        divShowWin = null;
                    })
                    divShowWin.on('ready-to-show', () => {
                    })

                }

            },
            // {
            //     label: 'Communicate Method',
            //     submenu: [
            //         {
            //             label: 'WebSocket',
            //             click() {
            //                 if (WebSocketWin != null) {
            //                     if (!WebSocketWin.isVisible()) {
            //                         WebSocketWin.show();
            //                     }
            //                     return;
            //                 }
            //                 WebSocketWin = new BrowserWindow({
            //                     width: 500,
            //                     height: 250,
            //                     title: "wsSetting",
            //                     resizable: process.env.NODE_ENV == "production" ? false : true,
            //                     webPreferences: { nodeIntegration: true, contextIsolation: false, enableRemoteModule: true },
            //                 });
            //                 WebSocketWin.loadURL(url.pathToFileURL(path.join(__dirname, "WebSocketParaSetting", "ws_paraSetting.html")).href);
            //                 //garbage collection
            //                 WebSocketWin.on('close', (e) => {
            //                     //这边需要其他方式进行释放
            //                     // e.preventDefault();
            //                     WebSocketWin.hide();
            //                     WebSocketWin = null;
            //                 })
            //                 WebSocketWin.on('ready-to-show', () => {
            //                     WebSocketWin.webContents.send('wsWin:Init');
            //                 })

            //             }
            //         },
            //     ]
            // },
            {
                label: 'Quit',
                accelerator: process.platform == "darwin" ? 'Command+Q' : 'Ctrl +Q',
                click() {
                    app.quit();
                }
            }

        ]
    },
]

if (process.env.NODE_ENV != "production") {
    menutemplate.push({
        label: 'Debug',
        submenu: [
            {
                label: 'Toggle DevTool',
                click(item, focusdWindow) {
                    focusdWindow.toggleDevTools();
                }
            }
        ]

    });
}
/////ipc 
// ipcMain.once("wsWin:ImgData", (e, img) => {
//     WebSocketWin.hide();
// });

// ipcMain.on("wsWin:ImgData", (e, img) => {
//     if (mainWindow != null) {
//         mainWindow.webContents.send('wsWin:ImgData', img);
//     }
// });

ipcMain.on('divShowWin:row', (e, row) => {
    mainWindow.webContents.send('divShowWin:row', row);
})

ipcMain.on('divShowWin:idxCol', (e, idxCol) => {
    mainWindow.webContents.send('divShowWin:idxCol', idxCol);
})

ipcMain.on('mainFrameImg:click', (e, imgId) => {
    console.log("Hey there...");
    if (WebSocketWin[imgId] == null) {

        let wsNewBrowser = new BrowserWindow({
            width: 500,
            height: 250,
            title: imgId.toString(),
            resizable: process.env.NODE_ENV == "production" ? false : true,
            webPreferences: { nodeIntegration: true, contextIsolation: false, enableRemoteModule: true },
        });
        wsNewBrowser.loadURL(url.pathToFileURL(path.join(__dirname, "WebSocketParaSetting", "ws_paraSetting.html")).href);
        //garbage collection
        wsNewBrowser.on('close', (e) => {
            e.preventDefault();
            wsNewBrowser.hide();
            //wsNewBrowser = null
        })
        wsNewBrowser.on('ready-to-show', () => {
            wsNewBrowser.webContents.send('wsWin:IDSet', imgId);
        })
        WebSocketWin[imgId] = wsNewBrowser;
    }
    else {
        WebSocketWin[imgId].show();
    }
})
ipcMain.on('wsWin:Connect', (e, data) => {
    mainWindow.webContents.send('wsWin:Connect', data);
})

ipcMain.on('wsWin:connectStatus', (e, data) => {
    var img_id = data.id;
    var say = data.say;
    if (WebSocketWin[img_id] != null) {
        var tar_wsWin = WebSocketWin[img_id];
        tar_wsWin.webContents.send('wsWin:connectStatus', data);
    }
})
    //  id: this.id, say: sayWhat })

///end ipc
