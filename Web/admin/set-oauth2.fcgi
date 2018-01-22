<!DOCTYPE html>
<html>
<head>
	<meta charset="utf-8">
	<title>微信 OAuth 2.0 设置 - %s</title>
	<link rel="stylesheet" href="css/pintuer.css">
	<link rel="stylesheet" href="../css/font-awesome.min.css">
	<link rel="stylesheet" href="css/admin.css">
	<script src="../js/jquery.min.js"></script>
	<script src="js/pintuer.js"></script>
</head>
<body>
	<div class="panel admin-panel" id="add">
	  <div class="panel-head"><strong><i class="fa fa-pencil-square-o"></i> 微信 OAuth 2.0 设置</strong></div>
	  <div class="body-content">
		<form method="post" class="form-x" action="set-oauth2.fcgi">
		  <div class="form-group">
			<div class="label">
			  <label><b>是否开启微信 OAuth 2.0 功能：</b>(提供微信绑定，微信登录，信息关联等功能。0-禁用, 1-启用)</label>
			</div>
			<div class="field">
			  <input type="text" class="input" name="ENABLE_OAUTH2" value="%s" />
			  <div class="tips"></div>
			</div>
		  </div>		
		  <div class="form-group">
			<div class="label">
			  <label><b>微信 OAuth 2.0 应用ID(AppID)：</b></label>
			</div>
			<div class="field">
			  <input type="text" class="input" name="OAUTH2_APPID" value="%s" />
			  <div class="tips"></div>
			</div>
		  </div>
		  <div class="form-group">
			<div class="label">
			  <label><b>微信 OAuth 2.0 应用密钥(Secret)：</b></label>
			</div>
			<div class="field">
			  <input type="text" class="input" name="OAUTH2_SECRET" value="%s" />
			  <div class="tips"></div>
			</div>
		  </div>
		  <div class="form-group">
			<div class="label">
			  <label></label>
			</div>
			<div class="field">
			  <button class="button bg-main fa fa-check-square-o" type="submit"> 提交</button>
			</div>
		  </div>
		</form>
	  </div>
	</div>
</body>
</html>