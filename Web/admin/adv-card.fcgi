<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta http-equiv="X-UA-Compatible" content="IE=edge">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" />
<title>广告轮播 - %s</title>
<link rel="stylesheet" href="css/pintuer.css">
<link rel="stylesheet" href="css/admin.css">
<script src="js/jquery.js"></script>
<script src="js/pintuer.js"></script>
</head>
<body>
	<div class="panel admin-panel" id="add">
	  <div class="panel-head"><strong><span class="icon-pencil-square-o"></span> 广告轮播 (留空则不显示广告)</strong></div>
	  <div class="body-content">
		<form method="post" class="form-x" action="">    
		  <div class="form-group">
			<div class="label">
			  <label>广告一：图片URL</label>
			</div>
			<div class="field">
			  <input type="text" class="input" name="CARD_AD_BANNER_1_IMG" value="%s" />
			  <div class="tips"></div>
			</div>
		  </div>
		  <div class="form-group">
			<div class="label">
			  <label>广告一：链接URL</label>
			</div>
			<div class="field">
			  <input type="text" class="input" name="CARD_AD_BANNER_1_URL" value="%s" />
			  <div class="tips"></div>
			</div>
		  </div>
		   <div class="form-group">
			<div class="label">
			  <label>广告二：图片URL</label>
			</div>
			<div class="field">
			  <input type="text" class="input" name="CARD_AD_BANNER_2_IMG" value="%s" />
			  <div class="tips"></div>
			</div>
		  </div>
		  <div class="form-group">
			<div class="label">
			  <label>广告二：链接URL</label>
			</div>
			<div class="field">
			  <input type="text" class="input" name="CARD_AD_BANNER_2_URL" value="%s" />
			  <div class="tips"></div>
			</div>
		  </div>
		  <div class="form-group">
			<div class="label">
			  <label></label>
			</div>
			<div class="field">
			  <button class="button bg-main icon-check-square-o" type="submit"> 提交</button>
			</div>
		  </div>
		</form>
	  </div>
	</div>
</body>
</html>