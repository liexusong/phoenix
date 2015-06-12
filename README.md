phoenix
=======
phoenix is a HTTP server for PHP extension

```php
<?php

function work_cb(PhxClient $client)
{
  $name = $client->getUriArg('name');

  $client->status(200);
  $client->sendHeader("connection: close");
  $client->sendBody('Hello: ' . $name);
  $client->end(); /* send response */
}

$phx = new Phoenix();
$phx->registerCallback('/work', 'work_cb');  /* register process callback function */
$phx->workers(10);                           /* create 10 worker process */
$phx->listen('127.0.0.1', 8080);
```
