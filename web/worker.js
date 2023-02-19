if ( 'function' === typeof importScripts) {
    importScripts('web.js');
    addEventListener('message', onMessage);
 
    function onMessage(event) { 
        console.log('Message received: ' + event.data);
        wrapped_worker(event.data.thread_num);
    }    
}