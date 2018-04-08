<?php

date_default_timezone_set('PRC');

define("PDODB_DRIVER", "mysql");
define("PDODB_SERVER_IP", "172.17.32.28");
define("PDODB_SERVER_PORT", "3306");
define("PDODB_USERNAME", "iedon3");
define("PDODB_PASSWORD", "iedon3");

class PDODB {
  private static $instance = NULL;
  private function __construct() { }
  private function __clone() { }
  public static function getInstance() {
    if (!self::$instance) {
      self::$instance = new PDO(PDODB_DRIVER . ":host=" . PDODB_SERVER_IP . ":" . PDODB_SERVER_PORT, PDODB_USERNAME, PDODB_PASSWORD);
      self::$instance->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    }
    return self::$instance;
  }
}

function write_log($log)
{
	$file  = './Logs/selfsvc_log.txt';
	$log = date("Y-m-d H:i:s") . " - [用户IP]: " . $_SERVER['REMOTE_ADDR'] . ":" . $_SERVER['REMOTE_PORT'] . ", " . $log . "\r\n";
	file_put_contents($file, $log, FILE_APPEND);
}

function get_current_sxm() {
	$curl = curl_init();
	curl_setopt($curl, CURLOPT_URL, 'http://home.iedon.com:9287/get_number');
	curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($curl, CURLOPT_TIMEOUT, 5);
	$data = curl_exec($curl);
	$httpCode = curl_getinfo($curl,CURLINFO_HTTP_CODE);
	curl_close($curl);
	if($httpCode != 200) {
		return "ERROR";
	} else {
		return $data;
	}
}

function do_find_pass() {
	
		$id = isset($_POST['xh']) ? $_POST['xh'] : "";
		$xm = isset($_POST['xm']) ? $_POST['xm'] : "";
		$sxm = isset($_POST['sxm']) ? $_POST['sxm'] : "";
		
		if($id == "" || $sxm == "" || $xm == "") {
			$arr = array ('code'=>'400','reason'=>'找回失败：提交数据不全');
			echo json_encode($arr);
			write_log("[ERR] 提交数据不全。");
			return;
		}
		
		$yzu_or_glxy = (substr($id, 2, 2) != '00') ? true : false;
		
		$current_sxm = get_current_sxm();
		if($current_sxm == "ERROR") {
			$arr = array ('code'=>'500','reason'=>'服务器错误');
			echo json_encode($arr);
			write_log("提交姓名: " . $xm . ", 提交学号: " . $id . ", [ERR] 服务器错误(SXM)");
			return;
		}
		
		if($current_sxm != $sxm) {
			$arr = array ('code'=>'400','reason'=>'时效码错误');
			echo json_encode($arr);
			write_log("提交姓名: " . $xm . ", 提交学号: " . $id . ", [ERR] 时效码错误。用户提交时效码为: " . $sxm . ", 此时正确值应为: " . $current_sxm);
			return;
		}
		
		$query_str = "SELECT name, password FROM " . ($yzu_or_glxy ? "yangda." : "guangling.") . "userinfo WHERE id=:id";
		$query = PDODB::getInstance()->prepare($query_str);
		$query->bindValue(':id', $id, PDO::PARAM_STR);
		$query->execute();
		$result = $query->fetch(PDO::FETCH_ASSOC);
		
		if(!isset($result['password']) || !isset($result['name'])) {
			$arr = array ('code'=>'404','reason'=>'没有您的记录');
			echo json_encode($arr);
			write_log("提交姓名: " . $xm . ", 提交学号: " . $id . ", [ERR] 找不到记录。");
		} else {
			if($result['name'] != $xm) {
				$arr = array ('code'=>'404','reason'=>'姓名不匹配');
				echo json_encode($arr);
				write_log("提交姓名: " . $xm . ", 提交学号: " . $id . ", [ERR] 姓名不匹配。用户提交姓名为: " . $xm . ", 此时正确值应为: " . $result['name']);
				return;
			}
			$ret = array();
			exec('"C:/iEdon Cluster/idn_gen_token_tools/gen_token.exe" ' . $id . ' ' . $result['password'], $ret, $status);
			if(!isset($ret[0]) || $status != 0) {
				$arr = array ('code'=>'500','reason'=>'服务器错误');
				echo json_encode($arr);
				write_log("提交姓名: " . $xm . ", 提交学号: " . $id . ", [ERR] 服务器错误(SHELL, Status: " . $status . " )");
				return;
			}
			$arr = array ('code'=>'200','reason'=>'OK','token'=>$ret[0],'school'=>($yzu_or_glxy ? "YZU" : "GLC"));
			echo json_encode($arr);
			write_log("提交姓名: " . $xm . ", 提交学号: " . $id . ", [OK] 受理成功。");
		}
}

function entry_point() {
	
	if( $_SERVER['REQUEST_METHOD'] == 'GET' ) {
		
		header('Content-Type: text/html; charset=utf-8');
		render_page();
		return;
		
	} elseif ($_SERVER['REQUEST_METHOD'] == 'POST') {
		
		header('Cache-Control: no-cache, no-store, must-revalidate');
		header('Pragma: no-cache');
		header('Content-Type: application/json; charset=utf-8');
		
		do_find_pass();
		return;
		
	} else {
		
		header('Cache-Control: no-cache, no-store, must-revalidate');
		header('Pragma: no-cache');
		header('Content-Type: application/json; charset=utf-8');
		
		$arr = array ('code'=>'400','reason'=>'错误的请求方法');
		echo json_encode($arr);
		
	}
	
}
entry_point();

function render_page() {
?>
	<!-- iEdon PASSWORD SELF-SERVICES -->
	<!DOCTYPE html>
	<html>
	  <head>
		<meta charset="utf-8">
		<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1" />
		<title>密码自助服务 ACCOUNT SELF-SERVICES</title>
		<link rel="stylesheet" href="https://urpsh.iedon.com/yzu/css/weui.min.css" />
		<link rel="stylesheet" href="https://urpsh.iedon.com/yzu/css/jquery-weui.min.css" />
		<link rel="stylesheet" href="https://urpsh.iedon.com/yzu/css/project.css" />
		<link rel="shortcut icon" href="https://urpsh.iedon.com/yzu/URPScoreHelper.ico" />
		<script type="text/javascript" src="https://urpsh.iedon.com/yzu/js/jquery.min.js"></script>
		<script type="text/javascript" src="https://urpsh.iedon.com/yzu/js/jquery-weui.min.js"></script>
		<style>
		  <!--.speed{height:20px;padding:0.15em 4px;font-size:12px;font-weight:600;line-height:15px;border-radius:2px;box-shadow:inset 0 -1px 0 rgba(27,31,35,0.12);}.speed.down{background-color:#cc317c;color:#FFF;}.speed.fast{background-color:#57db92;color:#FFF}.speed.intranet{background-color:#a65bd5;color:#FFF}.speed.slow{background-color:#cea40b;color:#FFF}.demos-sub-title{text-align:center;color:#888;font-size:14px}.demos-content-padded{padding:15px}.demos-second-title{text-align:center;font-size:24px;color:#3cc51f;font-weight:400;margin:0 15%}-->
		</style>
	  </head>
	  <body>
		<div id="container">
		  <header class="demos-header">
			<h1 class="demos-title" id="title">密码自助</h1>
			<div class="weui-cells__title status">Account Self-services</div></header>
			<div class="content">
				<div id="i_notice"><i class="fa fa-warning"></i>本自助服务不能保证一定成功找回密码。只有&nbsp;<b style="color:#cc2a2a">成功登录过掌上教务的用户</b>&nbsp;才有机会找回。操作成功后，将直接登录，请自行修改新密码。</div>
				<form id="ajax_submit" data-ajax-submit="selfsvc.php" class="weui-cells weui-cells_form">
					<div class="signbox">
						<div class="weui-cells__title"><span id="status_txt"></span></div>
						<div id="find_pass"></div>
					</div>	
					<div class="weui-btn-area">
						<button id="i_submit" type="button" class="weui-btn weui-btn_primary">
							<i class="fa fa-check-square-o"></i>找回密码
						</button>
					</div>
				</form>
			</div>
		</div>
		<div class="weui-footer">
				<p class="weui-footer__text">IP: <?php echo $_SERVER['REMOTE_ADDR'] . ":" . $_SERVER['REMOTE_PORT']; ?></p>
				<p class="weui-footer__text">你在本页面的所有操作将被监视并记录！</p>
				<p class="weui-footer__text">时效码电话绝不收任何费用，由运营商计入市话套餐时长。</p>
				<p class="weui-footer__text">时效码电话若无自动应答，说明服务暂不可用。</p>
				<br />
		</div>
		<script type="text/javascript">
			var HTML = '<div class="weui-cell"><div class="weui-cell__hd"><label class="weui-label">姓名</label></div><div class="weui-cell__bd"><input class="weui-input"id="i_xm"name="xm"type="text"placeholder="输入姓名"value=""></div></div><div class="weui-cell"><div class="weui-cell__hd"><label class="weui-label">学号</label></div><div class="weui-cell__bd"><input class="weui-input"id="i_xh"name="xh"type="text"placeholder="输入学号"value=""></div></div><div class="weui-cell"><div class="weui-cell__hd"><label class="weui-label">点击拨打</label></div><div class="weui-cell__bd"><a href="tel:18052637607"class="weui-input">180-5263-7607</a></div></div><div class="weui-cell"><div class="weui-cell__hd"><label class="weui-label">时效码</label></div><div class="weui-cell__bd"><input class="weui-input"name="sxm"id="i_sxm"type="text"placeholder="输入时效码"></div></div><label for="weuiAgree"class="weui-agree"><input id="weuiAgree"type="checkbox"class="weui-agree__checkbox"checked="checked"><span class="weui-agree__text">&nbsp;同意服务条款</span></label>';
			document.getElementById("status_txt").innerText = '请填写信息：';
			document.getElementById("find_pass").innerHTML = HTML;
			function ajax_submit() {
				var href = $("#ajax_submit").attr("data-ajax-submit");
				if (href != undefined) {
					$.ajax({
						url: href,
						type: "POST",
						contentType: "application/x-www-form-urlencoded",
						dataType: "json",
						data: $('#ajax_submit').serialize(),
						beforeSend: function(request) {
							$('#i_submit').hide();
							document.getElementById("status_txt").innerText = "正在提交请求...";
							document.getElementById("find_pass").innerHTML = '<center><img src="loading.gif" width="30%" height="30%" /><p style="font-size:20px;font-weight:bold;margin-top:10px">稍安勿躁...跳一跳</p></center>';
						},
						error: function(request) {
							document.getElementById("status_txt").innerText = "请求失败，稍后再来";
							document.getElementById("find_pass").innerHTML = '<center><img src="error.jpg" width="50%" height="25%" /><p style="font-size:20px;font-weight:bold;margin-top:10px">请求失败，pogai~</p></center>';
							$.toast("请求失败，请重试","text");
						},
						success: function(data) {
							if(data.code != 200) {
								document.getElementById("status_txt").innerText = "很遗憾，找回失败了";
								document.getElementById("find_pass").innerHTML = '<center><img src="error.jpg" width="50%" height="25%" /><p style="font-size:20px;font-weight:bold;margin-top:10px">' + data.reason + '</p></center>';
								$.toast(data.reason,"text");
								return;
							}
							$.toast("密码找回成功！","text");
							$.toptip("密码找回成功，正在登录...", 2000, 'success');
							if(data.school == "GLC") {
								window.location.href="https://urpsh.iedon.com/glxy/index.fcgi?token=" + data.token;
							} else {
								window.location.href="https://urpsh.iedon.com/yzu/index.fcgi?token=" + data.token;
							}
						}
					});
				}
			}
			$("#i_submit").on("click", function(e) {
				var r1 = document.getElementById("i_xh");
				var r2 = document.getElementById("i_sxm");
				var r3 = document.getElementById("i_xm");
				var r4 = document.getElementById("weuiAgree");
				if(r1 != undefined && r1.value == "")
				{
					$.toast("请输入学号","cancel");
					return false;
				}
				if(r2 != undefined && r2.value == "")
				{
					$.toast("请输入时效码","cancel");
					return false;
				}
				if(r3 != undefined && r3.value == "")
				{
					$.toast("请输入姓名","cancel");
					return false;
				}
				if(r4 != undefined && !r4.checked)
				{
					$.toast("请同意条款","cancel");
					return false;
				}
				$.toptip("正在提交请求...", 2000, 'success');
				ajax_submit();
				return true;
			});
		</script>
		</body>
	</html>
<?php
}

?>