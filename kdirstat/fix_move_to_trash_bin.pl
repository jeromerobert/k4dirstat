#!/usr/bin/perl
#
# Replace ~/KDesktop/Trash to %t
#
while( <> )
{
    s:~?\S*/\S*Trash\S*:%t: if ( /^\s*command\s*=\s*kfmclient\s+move/ );
    print $_;
}
