<!DOCTYPE html>
<html>
<head>
	<meta charset="utf-8">
	<title>交流讨论 - %s</title>
	<link rel="stylesheet" href="css/pintuer.css">
	<link rel="stylesheet" href="../css/font-awesome.min.css">
	<link rel="stylesheet" href="css/admin.css">
	<script src="../js/jquery.min.js"></script>
	<script src="js/pintuer.js"></script>
</head>
<body>
	<div class="panel admin-panel" id="add">
	  <div class="panel-head"><strong><i class="fa fa-pencil-square-o"></i> 交流讨论 (全部留空则关闭交流讨论功能)</strong></div>
	  <div class="body-content">
		<form method="post" class="form-x" action="">
		  <div class="form-group">
			<div class="label">
			  <label><b>页面内容：</b>(可以使用 &lt;div&gt; 内允许使用的 HTML 标签)</label>
			</div>
			<div class="field">
			  <textarea class="input" name="DISCUSSION_PAGE_CONTENT">%s</textarea>
			  <div class="tips"></div>
			</div>
		  </div>
		  <div class="form-group">
			<div class="label">
			  <label><b>第三方评论代码：</b>(本系统需要使用第三方提供的评论系统功能，展示在页面内容下方)</label>
			</div>
			<div class="field">
			  <textarea class="input" name="DISCUSSION_PAGE_CODE">%s</textarea>
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