<!DOCTYPE html>
<html>
<head>
	<meta charset="utf-8">
	<title>系统信息 - %s</title>
	<link rel="stylesheet" href="css/pintuer.css">
	<link rel="stylesheet" href="../css/font-awesome.min.css">
	<link rel="stylesheet" href="css/admin.css">
	<script src="../js/jquery.min.js"></script>
	<script src="js/pintuer.js"></script>
	<style>
		.label label{padding-left:20px;font-family:"Consolas"}
		.label label a{color:#1089ba}
	</style>
</head>
<body>
	<div class="panel admin-panel">
	  <div class="panel-head"><strong><i class="fa fa-pencil-square-o"></i> %s 系统信息</strong></div>
	  <div class="body-content"> 
	  	  <div class="form-group">
			<div class="label">
			  <label><b>用户数量：</b></label>
			</div>
			<div class="field">
				<div class="label">
				  <label>%d</label>
				</div>
			</div>
		  </div>
		  <div class="form-group">
			<div class="label">
			  <label><b>查询计数器：</b></label>
			</div>
			<div class="field">
				<div class="label">
				  <label>%d</label>
				  <label><a href="javascript:if(confirm('确实要重置查询计数器吗？'))location='info.fcgi?act=reset_query_counter'">重置</a></label>
				</div>
			</div>
		  </div>
		  <div class="form-group">
			<div class="label">
			  <label><b>Web 服务器：</b></label>
			</div>
			<div class="field">
				<div class="label">
				  <label>%s</label>
				</div>
			</div>
		  </div>
		   <div class="form-group">
			<div class="label">
			  <label><b>MySQL 信息：</b></label>
			</div>
			<div class="field">
				<div class="label">
				  <label>MySQL Connector 库版本：%s</label>
				  <label>服务器版本：%s</label>
				</div>
			</div>
		  </div>
		  <div class="form-group">
			<div class="label">
			  <label><b>程序版本：</b></label>
			</div>
			<div class="field">
				<div class="label">
				  <label>%s %s</label>
				</div>
			</div>
		  </div>
		  <div class="form-group">
			<div class="label">
			  <label><b>版权信息：</b></label>
			</div>
			<div class="field">
				<div class="label">
				  <label>%s</label>
				</div>
			</div>
		  </div>
	  </div>
	</div>
</body>
</html>