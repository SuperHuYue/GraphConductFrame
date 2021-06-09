var now_ind = 0;

var header_list = ["姓名", "性别", "年龄"];

var data_list = [
    {
        '姓名': '胡越',
        '性别': '男',
        '年龄': '31'
    },
    {
        '姓名': '李富贵',
        '性别': '女',
        '年龄': '25'
    },
    {
        '姓名': '胡甜奕',
        '性别': '女',
        '年龄': '1'
    },
    {
        '姓名': '李狗蛋',
        '性别': '女',
        '年龄': '25'
    },
    {
        '姓名': '胡越',
        '性别': '男',
        '年龄': '31'
    },
    {
        '姓名': '李富贵',
        '性别': '女',
        '年龄': '25'
    },
    {
        '姓名': '胡甜奕',
        '性别': '女',
        '年龄': '1'
    },
    {
        '姓名': '李狗蛋',
        '性别': '女',
        '年龄': '25'
    },
    {
        '姓名': '胡越',
        '性别': '男',
        '年龄': '31'
    },
    {
        '姓名': '李富贵',
        '性别': '女',
        '年龄': '25'
    },
    {
        '姓名': '胡甜奕',
        '性别': '女',
        '年龄': '1'
    },
    {
        '姓名': '李狗蛋',
        '性别': '女',
        '年龄': '25'
    },
    {
        '姓名': '胡越',
        '性别': '男',
        '年龄': '31'
    },
    {
        '姓名': '李富贵',
        '性别': '女',
        '年龄': '25'
    },
    {
        '姓名': '胡甜奕',
        '性别': '女',
        '年龄': '1'
    },
    {
        '姓名': '李狗蛋',
        '性别': '女',
        '年龄': '25'
    },
    {
        '姓名': '胡越',
        '性别': '男',
        '年龄': '31'
    },
    {
        '姓名': '李富贵',
        '性别': '女',
        '年龄': '25'
    },
    {
        '姓名': '胡甜奕',
        '性别': '女',
        '年龄': '1'
    },
    {
        '姓名': '李狗蛋',
        '性别': '女',
        '年龄': '25'
    },
    {
        '姓名': '胡越',
        '性别': '男',
        '年龄': '31'
    },
    {
        '姓名': '李富贵',
        '性别': '女',
        '年龄': '25'
    },
    {
        '姓名': '胡甜奕',
        '性别': '女',
        '年龄': '1'
    },
    {
        '姓名': '李狗蛋',
        '性别': '女',
        '年龄': '25'
    },
];

function buildTable(father_name, table_name, header, data) {
    var father = document.getElementById(father_name);
    var table = document.createElement('table');
    table.style.border = "solid";
    table.style.borderCollapse = "collapse"
    table.style.borderSpacing = "0px"
    table.id = table_name;
    //添加头
    var header_row = document.createElement('tr');
    for (let i of header.values()) {
        var header_slot = document.createElement('th');
        header_slot.innerText = i;
        header_slot.style.borderStyle = "solid"
        // header_slot.style.borderCollapse = "collapse"
        // header_slot.style.borderSpacing = "0";
        header_row.appendChild(header_slot)
    }
    table.appendChild(header_row)
    //添加数据
    for (let i of data.values()) {
        var row = document.createElement('tr');
        for (let key of header.values()) {
            var slot = document.createElement('td');
            var data = undefined;
            if (key in i) {
                data = i[key];
            }
            slot.innerText = data;
            slot.style.borderStyle = "solid"
            slot.style.minWidth = "50px"
            slot.style.textAlign = "center"
            row.appendChild(slot);
        }
        table.appendChild(row);
    }
    father.appendChild(table);
    setInterval(() => {
        var table_root = father
        var children = table_root.children[0]//tbody
        var tr_nodes = children.children;
        if (!now_ind) {
            now_ind = 1;
        }
        else {
            now_ind = 0;
        }
        for (let i = 0; i < tr_nodes.length; ++i) {
            //偶数对象
            let row = tr_nodes[i]
            if (i % 2) {
                row.style.backgroundColor = (now_ind == 0) ? "pink" : "yellow"
            } else {
                row.style.backgroundColor = (now_ind == 0) ? "yellow" : "pink";
            }
        }
    }, 100)
}

buildTable('TreeViewOne', 'test_table', header_list, data_list);






