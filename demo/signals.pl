
$SIG{ALRM} = sub {
	       print "alarm went off\n";
             };

alarm 2;
sleep 5;
