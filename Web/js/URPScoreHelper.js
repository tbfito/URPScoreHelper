function get_captcha() {
	var obj = document.getElementById("login_captcha");
	if(obj == undefined)
		return;
	$.ajax({
		type: "GET",
		url: "captcha.fcgi",
		dataType : "text",
		beforeSend: function() {
			obj.src = "img/loading.gif";
		},
		success: function(data) {
			if(data == "LOGGED-IN")
			{
				$.toast("�ѵ�¼��, ������ת", "text");
				window.location.href = "main.fcgi";
			}
			else if(data == "REQUEST-FAILED")
			{
				$.toast("��֤���ȡʧ�ܣ�ѧԺϵͳ���ܷ�������", "text");
				obj.src = "img/refresh.png";
			}
			else
			{
				obj.src = data;
			}
		},
		error: function() {
			obj.src = "img/refresh.png";
		}
	})
}
function autofill() {
	obj = document.getElementById("i_xh");
	if(obj.value=="")
	{
		$.toast("ѧ�Ż�û����","cancel");
		return false;
	}
	$.toptip("���Զ��������룬������ѧ����ͬ������", 3000, 'success');
	document.getElementById("i_mm").value = obj.value;
}
function checkRows() {
	var rows = document.getElementById("i_xh").value.split(/\r?\n|\r/).length;
	document.getElementById("i_xhhs").innerHTML = rows;
}
function getcharnum() {
	var nums = document.getElementById("i_jxpg").value.length;
	document.getElementById("i_xhhs").innerHTML = nums;
}
function logout() {
		$.confirm("ȷ��Ҫ�˳�ϵͳ��", function() {
			$.showLoading("���ڵǳ�...");
			window.location.href = "index.fcgi?act=logout";
		}, function() {
		});
}
function releaseAssoc(id) {
	$.confirm("ȷ��Ҫ���ѧ����QQ�ŵĹ�����", function() {
		$.showLoading("���ڽ��...");
		window.location.href = "OAuth2Assoc.fcgi?release="+id;
	}, function() {
	});
}
function check_password() {
		var r1 = document.getElementById("i_xhs").value;
		if(r1=="")
		{
			$.toast("������������","cancel");
			return false;
		}
		var patrn=/^(\w){6,12}$/;
		if (!patrn.exec(r1)){
			$.toast("ֻ������6-12����ĸ�����֡��»���", "text");
			return false;
		}
		if(!document.getElementById("i_chk").checked){
			$.toast("�����ұ߿�����ȷ���������� :-)", "text");
			return false;
		}
		$.showLoading("�����޸�");
		return true;
}
$(function () {
	get_captcha();
		$(document).on("click", "#i_submit", function(e) {
				document.oncontextmenu=new Function("event.returnValue=false;");
				   var r1 = document.getElementById("i_xh");
				   var r2 = document.getElementById("i_mm");
				   var r3 = document.getElementById("i_yzm");
				   var r4 = document.getElementById("weuiAgree");
				   var r5 = document.getElementById("i_jxpj");
				if(r1 != undefined && r1.value=="")
				{
					$.toast("ѧ�Ż�û����","cancel");
					return false;
				}
				if(r2 != undefined && r2.value=="")
				{
					$.toast("���뻹û����","cancel");
					return false;
				}
				if(r3 != undefined && r3.value=="")
				{
					$.toast("��֤�뻹û����","cancel");
					return false;
				}
				if(r4 != undefined && !r4.checked)
				{
					$.toast("����ͬ������Ŷ","cancel");
					return false;
				}
				if(r5 != undefined && r5.value=="")
				{
					$.toast("ûд�������۰�","cancel");
					return false;
				}
				$.showLoading("���Ժ�");
		});
		$(".weui-grid.js_grid").not("#logout").on("click", function(e) {
			$.showLoading("���Ժ�");
		});
		$(".return").on("click", function(e) {
			$.showLoading("���Ժ�");
		});
});