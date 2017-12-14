<!DOCTYPE html>
<html>
<head>
	<meta charset="utf-8">
	<title>维护模式 - %s</title>
	<link rel="stylesheet" href="css/pintuer.css">
	<link rel="stylesheet" href="../css/font-awesome.min.css">
	<link rel="stylesheet" href="css/admin.css">
	<script src="../js/jquery.min.js"></script>
	<script src="js/pintuer.js"></script>
</head>
<body>
	<div class="panel admin-panel" id="add">
	  <div class="panel-head"><strong><i class="fa fa-pencil-square-o"></i> 维护模式 (留空则不处于维护模式，非空则会显示维护公告)</strong></div>
	  <div class="body-content">
		<form method="post" class="form-x" action="">
		  <div class="form-group">
			<div class="label">
			  <label><b>维护公告：</b>(可以使用 &lt;div&gt; 内允许使用的 HTML 标签)</label>
			</div>
			<div class="field">
			  <textarea class="input" name="SITE_MAINTENANCE">%s</textarea>
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