#!/usr/bin/perl -w -p
#
# Replace ~/KDesktop/Trash to %t
#
s:~?\S*/\S*Trash\S*:%t: if ( /^command\s*=/ )
