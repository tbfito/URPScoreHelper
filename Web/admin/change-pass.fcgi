<!DOCTYPE html>
<html lang="zh-cn">
<head>
	<meta charset="utf-8">
	<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
	<title>管理员帐号修改 - %s</title>
	<link rel="stylesheet" href="css/pintuer.css">
	<link rel="stylesheet" href="../css/font-awesome.min.css">
	<link rel="stylesheet" href="css/admin.css">
	<script src="../js/jquery.min.js"></script>
	<script src="js/pintuer.js"></script>
</head>
<body>
	<div class="panel admin-panel">
	  <div class="panel-head"><strong><i class="fa fa-key"></i> 管理员帐号修改</strong></div>
	  <div class="body-content">
		<form method="post" class="form-x" action="change-pass.fcgi">
		  <div class="form-group">
			<div class="label">
			  <label for="sitename">管理员帐号：</label>
			</div>
			<div class="field">
				<input type="text" class="input" id="musr" name="ADMIN_USER_NAME" size="16" placeholder="请输入管理员帐号名" data-validate="required:请输入管理员帐号名" value="%s" autofocus /> 
			</div>
		  </div>      
		  <div class="form-group">
			<div class="label">
			  <label for="sitename">原始密码：</label>
			</div>
			<div class="field">
			  <input type="password" class="input" id="mpass" name="ORIG_PASSWORD" size="16" placeholder="请输入原始密码" data-validate="required:请输入原始密码" />       
			</div>
		  </div>      
		  <div class="form-group">
			<div class="label">
			  <label for="sitename">新密码：</label>
			</div>
			<div class="field">
			  <input type="password" class="input" name="ADMIN_PASSWORD" size="16" placeholder="请输入新密码" data-validate="required:请输入新密码,length#>=5:新密码不能小于5位" />         
			</div>
		  </div>
		  <div class="form-group">
			<div class="label">
			  <label for="sitename">确认新密码：</label>
			</div>
			<div class="field">
			  <input type="password" class="input" name="RENEW_PASSWORD" size="16" placeholder="请再次输入新密码" data-validate="required:请再次输入新密码,repeat#ADMIN_PASSWORD:两次输入的密码不一致" />          
			</div>
		  </div>
		  <div class="form-group">
			<div class="label">
			  <label></label>
			</div>
			<div class="field">
			  <button class="button bg-main fa fa-check-square-o" type="submit"> 修改</button>   
			</div>
		  </div>      
		</form>
	  </div>
	</div>
</body>
</html>