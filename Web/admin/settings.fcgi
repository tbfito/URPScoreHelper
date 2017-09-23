<!DOCTYPE html>
<html lang="zh-cn">
<head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" />
    <title>站点设置 - %s</title>  
    <link rel="stylesheet" href="css/pintuer.css">
    <link rel="stylesheet" href="css/admin.css">
    <script src="../js/jquery.min.js"></script>
    <script src="js/pintuer.js"></script>  
</head>
<body>
<div class="panel admin-panel">
  <div class="panel-head"><strong><span class="icon-pencil-square-o"></span> 站点设置</strong></div>
  <div class="body-content">
    <form method="post" class="form-x" action="settings.fcgi">
      <div class="form-group">
        <div class="label">
          <label>站点名称：</label>
        </div>
        <div class="field">
          <input type="text" class="input" name="APP_NAME" value="%s" />
          <div class="tips"></div>
        </div>
      </div>
	   <div class="form-group">
        <div class="label">
          <label>站点副标题：(站点名称后的副标题)</label>
        </div>
        <div class="field">
          <input type="text" class="input" name="SECONDARY_TITLE" value="%s" />
          <div class="tips"></div>
        </div>
      </div>
	    <div class="form-group">
        <div class="label">
          <label>站点描述：</label>
        </div>
        <div class="field">
          <textarea class="input" name="APP_DESCRIPTION" style="height:80px">%s</textarea>
          <div class="tips"></div>
        </div>
      </div>
      <div class="form-group">
        <div class="label">
          <label>站点关键字：(多个关键字可以使用英文半角逗号隔开)</label>
        </div>
        <div class="field">
          <textarea class="input" name="APP_KEYWORDS">%s</textarea>
          <div class="tips"></div>
        </div>
      </div>
      <div class="form-group">
        <div class="label">
          <label>教务系统URL：(URP系统服务端URL，末尾不加"/"。)</label>
        </div>
        <div class="field">
          <input type="text" class="input" name="SERVER_URL" value="%s" />
        </div>
      </div>
      <div class="form-group">
        <div class="label">
          <label>爬虫UA：(设置爬虫UA名称，当教务系统封杀自定义UA时，这里应该伪装为普通浏览器的标识。)</label>
        </div>
        <div class="field">
          <input type="text" class="input" name="USER_AGENT" value="%s" />
          <div class="tips"></div>
        </div>
      </div>
	   <div class="form-group">
        <div class="label">
          <label>爬虫连接超时：(单位：秒。推荐2，若数值过大且遇到教务系统无法访问时会遇到挂起等待问题)</label>
        </div>
        <div class="field">
          <input type="text" class="input" name="CURL_TIMEOUT" value="%d" />
          <div class="tips"></div>
        </div>
      </div>
      <div class="form-group">
        <div class="label">
          <label>是否使用代理：(0-禁用,1-启用)</label>
        </div>
        <div class="field">
          <input type="text" class="input" name="CURL_USE_PROXY" value="%d" />
          <div class="tips"></div>
        </div>
      </div>
      <div class="form-group">
        <div class="label">
          <label>代理URL：(支持：SOCKS4, SOCKS5, SOCKS5h, HTTP, HTTPS。格式：scheme://usr:pwd@host.suffix)</label>
        </div>
        <div class="field">
          <input type="text" class="input" name="CURL_PROXY_URL" value="%s" />
          <div class="tips"></div>
        </div>
      </div>
      <div class="form-group">
        <div class="label">
          <label>OAuth2 应用ID(AppID)：</label>
        </div>
        <div class="field">
          <input type="text" class="input" name="OAUTH2_APPID" value="%s" />
          <div class="tips"></div>
        </div>
      </div>
      <div class="form-group">
        <div class="label">
          <label>OAuth 应用密钥(Secret)：</label>
        </div>
        <div class="field">
          <input type="text" class="input" name="OAUTH2_SECRET" value="%s" />
          <div class="tips"></div>
        </div>
      </div>
      <div class="form-group">
        <div class="label">
          <label>页面尾部文字：(版权说明或其他自定义文本)</label>
        </div>
        <div class="field">
          <input type="text" class="input" name="FOOTER_TEXT" value="%s" />
          <div class="tips"></div>
        </div>
      </div>
      <div class="form-group">
        <div class="label">
          <label>统计代码：(用于放置第三方统计的代码)</label>
        </div>
        <div class="field">
		  <textarea class="input" name="ANALYSIS_CODE">%s</textarea>
          <div class="tips"></div>
        </div>
      </div>
	  <div class="form-group">
        <div class="label">
          <label></label>
        </div>
        <div class="field">
          <button class="button bg-main icon-check-square-o" type="submit"> 保存设定</button>
        </div>
      </div>
    </form>
  </div>
</div>
</body></html>