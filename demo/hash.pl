
sub dump_hash {
   my($h) = @_; my($k);
   print "\$hash = {\n";
   foreach $k (keys %{$h}) {
      print "  '$k' => '", $h->{$k}, "',\n";
   }
   print "};\n";
}

%h = ();

if (!%h) {
   print "hash is empty\n";
}

$h{"alpha"} = 1;
$h{"beta"} = 2;

dump_hash(\%h);

print "alpha = ", $h{"alpha"}, "\n";

$h{"beta"} = "example #2";

print "beta = ", $h{"beta"}, "\n";

if (!defined $h{"bogus"}) {
   print "'bogus' is not in the hash\n";
}
