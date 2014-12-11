phoenix
=======
phoenix is a HTTP server for PHP extension

```php
<?php

function work_cb($request, $response)
{
  $name = $request->get('name');
  
  $response->status(200);
  $response->send_header("connection: close");
  $response->send_body('Hello: ' . $name);
}

$ph = new Phoenix();

$ph->register_callback('/work', 'work_cb');

$ph->listen('127.0.0.1', 8080);
```
