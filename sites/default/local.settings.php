<?php

// Fast 404 pages.
$conf['404_fast_paths_exclude'] = '/\/(?:styles)\//';
$conf['404_fast_paths'] = '/\.(?:txt|png|gif|jpe?g|css|js|ico|swf|flv|cgi|bat|pl|dll|exe|asp)$/i';
$conf['404_fast_html'] = '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML+RDFa 1.0//EN" "http://www.w3.org/MarkUp/DTD/xhtml-rdfa-1.dtd"><html xmlns="http://www.w3.org/1999/xhtml"><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL "@path" was not found on this server.</p></body></html>';
$conf['file_temporary_path'] = '/tmp';

$databases['default']['default'] = array(
  'driver' => 'mysql',
  'database' => 'db543256_1',
  'username' => 'root',
  'port' => '3306',
  'password' => 'coeus123',
  'host' => 'localhost',
  'prefix' => '',
);
$update_free_access = TRUE;

ini_set('display_errors', 1);
ini_set('display_startup_errors', 0);
error_reporting(E_ALL);

ini_set('memory_limit','512M');
