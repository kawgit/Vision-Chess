if ( 'function' === typeof importScripts) {
    importScripts('web.js');
    addEventListener('message', onMessage);
 
    function onMessage(event) { 
        console.log('Message received: ' + event.data);
        setTimeout(event.data, function(){wrapped_stop(event.data.max_time);});
    }    
}