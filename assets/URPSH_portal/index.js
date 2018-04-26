
var servers = 0;
var ready = false;
var _3rd_party = GetQS("3rd_party");
var marqueeContent = new Array();
var marqueeInterval = new Array(); 
var marqueeId = 0; 
var marqueeDelay = 2000; 
var marqueeHeight = 20; 
document.getElementById("login").innerText = "刷新服务器 Refresh";
document.getElementById("status_txt").innerText = '正在连接登录服务器...'; 

function get_server_list() {
	$.ajax({
		type: "GET",
		url: "https://api.iedon.com/urpsh",
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
				
				if(data.notices.length > 0) {
					for(var j in data.notices){
						marqueeContent[j] = '<a href="' + data.notices[j].link + '" target="_blank">' + data.notices[j].content + '</a>';
					}
					initMarquee();
					document.getElementById("i_notice").style.display="block";
				}
				
				ready = true;
				document.getElementById("login").innerText = "进入 Login";
			}
		},
		beforeSend: function() {
			document.getElementById("sound").load();
			document.getElementById("sound").play();
			document.getElementById("login").innerText = "刷新服务器 Refresh";
			document.getElementById("status_txt").innerText = '正在连接登录服务器...'; 
			$.toptip("正在刷新服务器列表...", 2000, 'success');
			document.getElementById("server_list").innerHTML = '<center><img src="loading.gif" width="30%" height="30%" /></center>';
		},
		error: function() {
			if(!ready) {
				document.getElementById("status_txt").innerText = '登录服务器炸了 /(ㄒoㄒ)/~~'; 
				document.getElementById("server_list").innerHTML = '<center><img src="error.jpg" width="50%" height="25%" /></center>';
				$.alert({title: '提示 Alert',text: '服务器列表加载失败<br />Unable to load server list',onOK: function () {return;}});
				document.getElementById("login").innerText = "刷新服务器 Refresh";
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
		$.toptip("正在刷新服务器列表...", 2000, 'success');
		get_server_list();
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
			document.getElementById("server_sel" + i).innerHTML += '&nbsp;&nbsp;<span class="speed down">停机维护</span>';
			continue;
		}
		if (speed == -1) {
			document.getElementById("server_sel" + i).innerHTML = '<i class="weui-icon-cancel"></i>' + document.getElementById("server_sel" + i).innerHTML;
			document.getElementById("server_sel" + i).innerHTML += '&nbsp;&nbsp;<span class="speed down">超时</span>';
		} else if (speed == -2) {
			document.getElementById("server_sel" + i).innerHTML = '<i class="weui-icon-download" style="transform: rotate(-90deg)"></i>' + document.getElementById("server_sel" + i).innerHTML;
			document.getElementById("server_sel" + i).innerHTML += '&nbsp;&nbsp;<span class="speed intranet">校园网</span>';
		} else {
			if (speed >= 1.5 && speed < 3) {
				document.getElementById("server_sel" + i).innerHTML = '<i class="weui-icon-download" style="color:#c0bc04;transform: rotate(-90deg)"></i>' + document.getElementById("server_sel" + i).innerHTML;
				document.getElementById("server_sel" + i).innerHTML += '&nbsp;&nbsp;<span class="speed slow">' + speed + "s 一般</span>";
			} else if (speed >= 3.0) {
				document.getElementById("server_sel" + i).innerHTML = '<i class="weui-icon-download" style="color:#cc317c;transform: rotate(-90deg)"></i>' + document.getElementById("server_sel" + i).innerHTML;
				document.getElementById("server_sel" + i).innerHTML += '&nbsp;&nbsp;<span class="speed down">' + speed + "s 拥堵</span>";
			} else {
				document.getElementById("server_sel" + i).innerHTML = '<i class="weui-icon-download" style="transform: rotate(-90deg)"></i>' + document.getElementById("server_sel" + i).innerHTML;
				document.getElementById("server_sel" + i).innerHTML += '&nbsp;&nbsp;<span class="speed fast">' + speed + "s 畅通</span>";
			}
		}
	}
}

function initMarquee() { 
	var str = marqueeContent[0]; 
	document.getElementById("i_notice").innerHTML = '<div id="marqueeBox" style="overflow:hidden;height:' + marqueeHeight + 'px" onmouseover="clearInterval(marqueeInterval[0])" onmouseout="marqueeInterval[0]=setInterval(\'startMarquee()\',marqueeDelay)"><div>' + str + '</div></div>'; 
	marqueeId++; 
	marqueeInterval[0] = setInterval("startMarquee()", marqueeDelay); 
}

function startMarquee() { 
	var str = marqueeContent[marqueeId];
	marqueeId++;
	if(marqueeId >= marqueeContent.length) marqueeId = 0;
	if(document.getElementById("marqueeBox").childNodes.length == 1) {
		var nextLine = document.createElement('div');
		nextLine.innerHTML = str;
		document.getElementById("marqueeBox").appendChild(nextLine);
	} else {
		document.getElementById("marqueeBox").childNodes[0].innerHTML = str;
		document.getElementById("marqueeBox").appendChild(document.getElementById("marqueeBox").childNodes[0]);
		document.getElementById("marqueeBox").scrollTop = 0;
	}
	clearInterval(marqueeInterval[1]);
	marqueeInterval[1] = setInterval("scrollMarquee()", 20);
}

function scrollMarquee() {
	document.getElementById("marqueeBox").scrollTop++;
	if(document.getElementById("marqueeBox").scrollTop%marqueeHeight == (marqueeHeight - 1)) {
		clearInterval(marqueeInterval[1]);
	}
}
