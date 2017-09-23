<!DOCTYPE html>
<html lang="zh-cn">
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" />
    <title>后台管理平台 - %s</title>  
    <link rel="stylesheet" href="css/pintuer.css">
    <link rel="stylesheet" href="css/admin.css">
	<link rel="shortcut icon" href="../URPScoreHelper.ico">
    <script src="../js/jquery.min.js"></script>   
</head>
<body style="background-color:#f2f9fd;">
<div class="header bg-main">
  <div class="logo margin-big-left fadein-top">
    <h1><img src="../URPScoreHelper.ico" class="radius-circle rotate-hover" height="50" alt="" />%s</h1>
  </div>
  <div class="head-l"><a class="button button-little bg-green" href="../" target="_blank"><span class="icon-home"></span> 前台首页</a>&nbsp;&nbsp;<a class="button button-little bg-red" href="login.fcgi?act=logout"><span class="icon-power-off"></span> 退出登录</a></div>
</div>
<div class="leftnav">
  <div class="leftnav-title"><strong><span class="icon-list"></span>菜单列表</strong></div>
  <h2><span class="icon-user"></span>基本设置</h2>
  <ul style="display:block">
    <li><a href="settings.fcgi" target="right"><span class="icon-caret-right"></span>站点设置</a></li>
    <li><a href="adv-card.fcgi" target="right"><span class="icon-caret-right"></span>广告轮播</a></li>
	<li><a href="change-pass.fcgi" target="right"><span class="icon-caret-right"></span>修改密码</a></li>
  </ul>   
</div>
<script type="text/javascript">
$(function(){
  $(".leftnav h2").click(function(){
	  $(this).next().slideToggle(200);	
	  $(this).toggleClass("on"); 
  })
  $(".leftnav ul li a").click(function(){
	    $("#a_leader_txt").text($(this).text());
  		$(".leftnav ul li a").removeClass("on");
		$(this).addClass("on");
  })
});
</script>
<ul class="bread">
  <li>请注意及时保存修改的信息。为确保服务稳定，建议避开高峰时期再修改设置。</li>
</ul>
<div class="admin">
  <iframe scrolling="auto" rameborder="0" src="settings.fcgi" name="right" width="100%%" height="100%%"></iframe>
</div>
</body>
</html>