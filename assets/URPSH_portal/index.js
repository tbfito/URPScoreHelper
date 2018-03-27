var servers = 0;
var ready = false;
var _3rd_party = GetQS("3rd_party");
function get_server_list() {
	$.ajax({
		type: "GET",
		url: "https://api.iedon.com/urpsh/server.php",
		dataType: "json",
		cache: false, 
		success: function(data) {
			if(data != null) {
				var html = '';
				for(var i in data.servers){
					html += '<label class="weui-cell weui-check__label" for="server_x' + i + '">';
					html += '<div class="weui-cell__bd"><p data-speed="' + data.servers[i].speed + '" data-url="' + data.servers[i].url + '" data-cn_name="' + data.servers[i].cn_name + '" data-en_name="' + data.servers[i].en_name + '" data-status="' + data.servers[i].status + '" id="server_sel' + i + '">' + data.servers[i].cn_name + '</p></div><div class="weui-cell__ft"><input onclick="javascript:clicked(' + i + ');" type="radio" class="weui-check" id="server_x' + i +'"/><span class="weui-icon-checked"></span></div></label>';
				}
				servers = i;
				document.getElementById("server_list").innerHTML = html;
				show_speed();
				clicked(0);
				document.getElementById("status_txt").innerText = '已选：'; 
				ready = true;
			}
		},
		beforeSend: function() {
			document.getElementById("server_list").innerHTML = '<center><img src="loading.gif" width="30%" height="30%" /></center>';
		},
		error: function() {
			if(!ready) {
				$.alert({title: '提示 Alert',text: '服务器列表加载失败<br />Unable to load server list',onOK: function () {return;}});
			}
		}
	})
}
get_server_list();
function GetQS(name) {
    var search = document.location.search;
    var pattern = new RegExp("[?&]" + name + "=([^&]+)", "g");
    var matcher = pattern.exec(search);
    var items = null;
    if (null != matcher) {
        try {
            items = decodeURIComponent(decodeURIComponent(matcher[1]));
        } catch (e) {
            try {
                items = decodeURIComponent(matcher[1]);
            } catch (e) {
                items = matcher[1];
            }
        }
    }
    return items;
}
if(_3rd_party != null) {
	document.getElementById("title").innerText = _3rd_party;
}
function enter_jw() {
	if(!ready) {
		$.alert({title: '提示 Alert',text: '服务器列表还未加载<br />Server list is not ready',onOK: function () {return;}});
		return;
	}
	for (var i = 0; i <= servers; i++) {
		if(document.getElementById("server_x" + i).checked) {
			if(document.getElementById("server_sel" + i).getAttribute("data-status") != "open") {
				$.alert({title: '提示 Alert',text: '服务器关闭或处于维护状态<br />Out of service',onOK: function () {return;}});
				return;
			}
			if(document.getElementById("server_sel" + i).getAttribute("data-speed") == -1) {
				$.alert({title: '提示 Alert',text: '服务器链路不佳，请稍后再试<br />Server is not stable',onOK: function () {return;}});
				return;
			}
			window.location.href = document.getElementById("server_sel" + i).getAttribute("data-url") + (_3rd_party != null ? "?3rd_party=" + _3rd_party : "");
			return;
		}
	}
}

function clicked(i) {
	for (var j = 0; j <= servers; j++) {
		document.getElementById("server_x" + j).checked = "";
	}
	document.getElementById("server_x" + i).checked = "checked";
	document.getElementById("notice").innerText = document.getElementById("server_sel" + i).getAttribute("data-cn_name") + " " + document.getElementById("server_sel" + i).getAttribute("data-en_name");
}

function show_speed() {
	for (var i = 0; i <= servers; i++) {
		var speed = document.getElementById("server_sel" + i).getAttribute("data-speed");
		if (document.getElementById("server_sel" + i).getAttribute("data-status") != "open") {
			document.getElementById("server_sel" + i).innerHTML = '<i class="weui-icon-cancel"></i>' + document.getElementById("server_sel" + i).innerHTML;
			document.getElementById("server_sel" + i).innerHTML += '&nbsp;&nbsp;<span class="speed down">维护</span>';
			continue;
		}
		if (speed == -1) {
			document.getElementById("server_sel" + i).innerHTML = '<i class="weui-icon-cancel"></i>' + document.getElementById("server_sel" + i).innerHTML;
			document.getElementById("server_sel" + i).innerHTML += '&nbsp;&nbsp;<span class="speed down">超时</span>';
		} else if (speed == -2) {
			document.getElementById("server_sel" + i).innerHTML = '<i class="weui-icon-download" style="transform: rotate(-90deg)"></i>' + document.getElementById("server_sel" + i).innerHTML;
			document.getElementById("server_sel" + i).innerHTML += '&nbsp;&nbsp;<span class="speed intranet">校园网</span>';
		} else {
			if (speed >= 1.2) {
				document.getElementById("server_sel" + i).innerHTML = '<i class="weui-icon-download" style="color:#c0bc04;transform: rotate(-90deg)"></i>' + document.getElementById("server_sel" + i).innerHTML;
				document.getElementById("server_sel" + i).innerHTML += '&nbsp;&nbsp;<span class="speed slow">' + speed + "s 忙</span>";
			} else {
				document.getElementById("server_sel" + i).innerHTML = '<i class="weui-icon-download" style="transform: rotate(-90deg)"></i>' + document.getElementById("server_sel" + i).innerHTML;
				document.getElementById("server_sel" + i).innerHTML += '&nbsp;&nbsp;<span class="speed fast">' + speed + "s 优</span>";
			}
		}
	}
}