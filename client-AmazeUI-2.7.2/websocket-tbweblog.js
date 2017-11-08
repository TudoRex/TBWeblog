var ws;
var log_index = 0;
var MAX_SCROLL_TOP = Math.pow(2, 30) - 1;

window.onload = function() {
    console.debug("window.onload");
    ws = new WebSocket("ws://127.0.0.1:9002");
    //连接建立成功onopen事件
    ws.onopen = function(e) {
        console.debug("ws.onopen");
        $("#status").text("服务已连接").css("color", "green");
    };
    //接收消息onmessage事件
    ws.onmessage = function(e) {   
        console.debug("ws.onmessage");
        var htmlstr;
        try {
            var tag_content_log = $('#content-log');
            //msg = JSON.parse(e.data);
            log_index++;
            var cur_date = new Date(Date.now());
            //htmlstr = log_index.toString() +'['+ cur_date.toLocaleString() +']'+ htmlEncode(e.data) + tag_content_log.scrollTop().toString() + '\n';
            htmlstr = log_index + '[' + cur_date.toLocaleString() + ']' + htmlEncode(e.data) + '\n';

            console.log(e.data);
            tag_content_log.append(htmlstr); //追加日志行
            tag_content_log.scrollTop(MAX_SCROLL_TOP);//Scroll latest line

        } catch (obj_exception) {
            //$("debug").innerHTML = "invalid message";
            alert(obj_exception.toString());
            console.debug(e.data);
            return false;
        }
    }
    //（服务端）连接断开时触发
    ws.onclose = function(e){
        console.debug("ws.onclose");
        $("#status").text('服务断开了').css("color", "red");
    };
};


    
//发送消息

function sendmsg() {
    ws.send($("ta").value);
}

function htmlEncode(value){
    //create a in-memory div, set it's inner text(which jQuery automatically encodes)
    //then grab the encoded contents back out.  The div never exists on the page.
    return $('<div/>').text(value).html();
  }
  
  function htmlDecode(value){
    return $('<div/>').html(value).text();
  }







//关闭页面时记得一定要关闭连接，否则服务端可能不会释放

window.onunload = function(){
    if (ws)
    {
        ws.close();
    }
};