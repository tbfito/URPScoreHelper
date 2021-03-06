<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>后台管理平台 - %s</title>  
    <link rel="stylesheet" href="css/pintuer.css">
	<link rel="stylesheet" href="../css/font-awesome.min.css">
    <link rel="stylesheet" href="css/admin.css">
	<link rel="shortcut icon" href="../URPScoreHelper.ico">
    <script src="../js/jquery.min.js"></script>   
</head>
<body style="background-color:#f2f9fd;">
	<div class="header bg-main">
	  <div class="logo margin-big-left fadein-top">
		<h1><img src="../URPScoreHelper.ico" class="radius-circle rotate-hover" height="50" alt="" />%s</h1>
	  </div>
	  <div class="head-l"><a class="button button-little bg-green" href="../" target="_blank"><i class="fa fa-home"></i> 前台首页</a>&nbsp;&nbsp;<a class="button button-little bg-red" href="login.fcgi?act=logout"><i class="fa fa-power-off"></i> 退出登录</a></div>
	</div>
	<div class="leftnav">
	  <div class="leftnav-title"><strong><i class="fa fa-list"></i>菜单列表</strong></div>
	  <h2><i class="fa fa-user"></i>设置</h2>
	  <ul style="display:block">
		<li><a href="settings.fcgi" target="right" class="on"><i class="fa fa-caret-right"></i>站点设置</a></li>
		<li><a href="homepage-notice.fcgi" target="right"><i class="fa fa-caret-right"></i>首页公告</a></li>
		<li><a href="set-discussion.fcgi" target="right"><i class="fa fa-caret-right"></i>交流讨论</a></li>
		<li><a href="adv-card.fcgi" target="right"><i class="fa fa-caret-right"></i>广告轮播</a></li>
		<li><a href="set-oauth2.fcgi" target="right"><i class="fa fa-caret-right"></i>微信 OAuth 2.0</a></li>
		<li><a href="change-pass.fcgi" target="right"><i class="fa fa-caret-right"></i>修改密码</a></li>
		<li><a href="find-user.fcgi" target="right"><i class="fa fa-caret-right"></i>查询用户</a></li>
		<li><a href="maintenance.fcgi" target="right"><i class="fa fa-caret-right"></i>维护模式</a></li>
		<li><a href="info.fcgi" target="right"><i class="fa fa-caret-right"></i>系统信息</a></li>
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
	  <li>请注意及时保存修改的信息，所有设置项内容的大小不能超过10KB。</li>
	</ul>
	<div class="admin">
	  <iframe scrolling="auto" rameborder="0" src="settings.fcgi" name="right" width="100%%" height="100%%"></iframe>
	</div>
</body>
</html>