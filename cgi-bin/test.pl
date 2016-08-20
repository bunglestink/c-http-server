#!/usr/bin/perl

$body = "<h2>Hello from <b>Perl</b>!!!</h2>";

print "Content-Length: " . length($body) . "\r\n";
print "Content-Type: text/html\r\n";
print "\r\n";
print $body;

