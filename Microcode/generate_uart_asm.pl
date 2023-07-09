use warnings;
use strict;

my %populate_bytes = (
2046 => 'D9',
2045 => '5D',
2043 => 'DB',
2039 => '5F',
);

my $start_offset = 0;
my $end_offset = 2047;
my $default_fill = 'FF';
my $current_fill;
my $bytes_written = 0;

open(OUTPUT,">",'gen_uart_2k.asm') or die "Can't open the output file!\n";

print OUTPUT "  CPU 1802\n";
print OUTPUT "\n";
print OUTPUT "  ORG \$0000\n";
print OUTPUT "\n";

$bytes_written = 0;

for (my $lcv = $start_offset; $lcv <= $end_offset; $lcv++) {

  if (exists($populate_bytes{$lcv})) {
    $current_fill = $populate_bytes{$lcv};
  }
  else {
    $current_fill = $default_fill;
  };

  print OUTPUT sprintf("  BYTE \$%s\n",$current_fill);

};

print OUTPUT "\n";
print OUTPUT "  END\n";
print OUTPUT "\n";

close(OUTPUT);

exit 0;