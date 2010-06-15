#!/usr/bin/env perl
use Cwd;
use File::Path;
use FileHandle;
use strict;


our (%MODE, %PG, %ENV_SET, %SUPPORTED_OS);


our $EXIT_SUCCESS=0;
our $EXIT_FAILURE=1;
our $INTERNET=0;

our $CP="cp "; #was causing a crash on MacOSX
our $SILENT=">/dev/null 2>/dev/null";
our $WEB_BASE="http://www.tcoffee.org";
our $TCLINKDB_ADDRESS="$WEB_BASE/Resources/tclinkdb.txt";
our $OS=get_os();
our $ROOT=&get_root();
our $CD=cwd();
our $CDIR=$CD;
our $HOME=$ENV{'HOME'};
our $CXX="g++";
our $CXXFLAGS="";

our $CPP="g++";
our $CPPFLAGS="";

our $CC="gcc";
our $CFLAGS="";

our $FC="f77";
our $FFLAGS="";

my $install="all";
my $default_update_action="no_update";
my @required_applications=("wget_OR_curl");
my @smode=("all", "clean", "install");

&initialize_PG();

my $cl=join( " ", @ARGV);
if ($#ARGV==-1 || ($cl=~/-h/) ||($cl=~/-H/) )
  {
     print "\n!!!!!!! ./install  t_coffee             --> installs t_coffee only";
     print "\n!!!!!!! ./install  all                  --> installs all the modes [mcoffee, expresso, psicoffee,rcoffee..]";
     print "\n!!!!!!! ./install  [mcoffee|rcoffee|..] --> installs the specified mode";
     print "\n!!!!!!! ./install  -h                   --> print usage\n\n";
     if ( $#ARGV==-1){exit ($EXIT_FAILURE);}
   }
     
if (($cl=~/-h/) ||($cl=~/-H/) )
  {
    my $m;
    print "\n\n!!!!!!! advanced mode\n";
    foreach $m ((keys (%MODE)),@smode)
      {
	print "!!!!!!!       ./install $m\n";
      }
    
    print "!!!!!!! ./install [target:package|mode|] [-update|-force|-no_question|-path=dir|-dis=dir|-no_root|-tclinkdb=file] [CC=|FCC=|CXX=|CFLAGS=|CXXFLAGS=]\n";
    print "!!!!!!! ./install clean    [removes all executables]\n";
    print "!!!!!!! ./install [optional:target] -update               [updates package already installed]\n";
    print "!!!!!!! ./install [optional:target] -force                [Forces recompilation over everything]\n";
    print "!!!!!!! ./install [optional:target] -no_question          [Do everything without question]\n";
    print "!!!!!!! ./install [optional:target] -root                 [Never ask the root password]\n";
    print "!!!!!!! ./install [optional:target] -path=/foo/bar/       [Final address for the distribution dir]\n";
    print "!!!!!!! ./install [optional:target] -dis=/foo/bar/        [Address where executables should be installed]\n";
    print "!!!!!!! ./install [optional:target] -tclinkdb=foo|update  [file containing all the packages to be installed]\n";
    print "!!!!!!! ./install [optional:target] -tclinkdb=foo|update  [file containing all the packages to be installed]\n";
    print "!!!!!!! ./install [optional:target] -clean                [clean everything]\n";
    
    print "!!!!!!! ./install install   [-path=/your/install/dir] [/usr/local/bin]\n";
    print "!!!!!!! mode:";
    foreach $m (keys(%MODE)){print "$m ";}
    print "\n";
    print "!!!!!!! Packages:";
    foreach $m (keys (%PG)){print "$m ";}
    print "\n";
    
    print "\n\n";
    exit ($EXIT_FAILURE);
  }



my (@argl)=($cl=~/(\S+=[^=]+)\s\w+=/g);
push (@argl, ($cl=~/(\S+=[^=]+\S)\s*$/g));

foreach $a (@argl)
  {
    if ( ($cl=~/CXX=(.*)/)){$CXX=$1;}
    if ( ($cl=~/-CC=(.*)/    )){$CC=$1;}
    if ( ($cl=~/-FC=(.*)/    )){$FC=$1;}
    if ( ($cl=~/-CFLAGS=(.*)/)){$CFLAGS=$1;}
    if ( ($cl=~/-CXXFLAGS=(.*)/)){$CXXFLAGS=$1;}
  }
our ($ROOT_INSTALL, $NO_QUESTION, $default_update_action,$BINARIES_ONLY,$force, $default_update_action, $INSTALL_DIR, $PLUGINS_DIR, $DISTRIBUTIONS,$tclinkdb, $proxy, $clean);
if ( ($cl=~/-root/)){$ROOT_INSTALL=1;}
if ( ($cl=~/-no_question/)){$NO_QUESTION=1;}
if ( ($cl=~/-update/)){$default_update_action="update";}
if ( ($cl=~/-binaries/)){$BINARIES_ONLY=1;}
if ( ($cl=~/-force/)){$force=1;$default_update_action="update"}
if ( ($cl=~/-exec=\s*(\S+)/)){$INSTALL_DIR=$1;}
if ( ($cl=~/-plugins=\s*(\S+)/)){$PLUGINS_DIR=$1;}
if ( ($cl=~/-distributions=\s*(\S+)/)){$DISTRIBUTIONS=$1;}

if ( ($cl=~/-tclinkdb=\s*(\S+)/)){$tclinkdb=$1;}
if ( ($cl=~/-proxy=\s*(\S+)/)){$proxy=$1;}
if ( ($cl=~/-clean/)){$clean=1;}
if ($tclinkdb){&update_tclinkdb ($tclinkdb);}

our $TCDIR=$ENV{DIR_4_TCOFFEE};
our $TCCACHE=$ENV{CACHE_4_TCOFFEE};
our $TCTMP=$ENV{CACHE_4_TCOFFEE};
our $TCM=$ENV{MCOFFEE_4_TCOFFEE};
our $TCMETHODS=$ENV{METHODS_4_TCOFFEE};
our $TCPLUGINS=$ENV{PLUGINS_4_TCOFFEE};
our $PLUGINS_DIR="";
our $INSTALL_DIR="";

&add_dir ($TCDIR="$HOME/.t_coffee");
&add_dir ($TCCACHE="$TCDIR/cache");
&add_dir ($TCTMP="$CDIR/tmp");
&add_dir ($TCM="$TCDIR/mcoffee");
&add_dir ($TCMETHODS="$TCDIR/methods");
&add_dir ($TCPLUGINS="$TCDIR/plugins/$OS");


our $BASE="$CD/bin";
our $BIN="$BASE/binaries/$OS";
our $DOWNLOAD_DIR="$BASE/download";
our $DOWNLOAD_FILE="$DOWNLOAD_DIR/files";
our $TMP="$BASE/tmp";

&add_dir($BASE);
&add_dir($BIN);
&add_dir($DOWNLOAD_DIR);
&add_dir($DOWNLOAD_FILE);
if (!$DISTRIBUTIONS){$DISTRIBUTIONS="$DOWNLOAD_DIR/distributions";}
&add_dir ($DISTRIBUTIONS);
&add_dir ($TMP);


if    (!$PLUGINS_DIR && !$ROOT_INSTALL){$PLUGINS_DIR=$TCPLUGINS;}
elsif (!$PLUGINS_DIR &&  $ROOT_INSTALL){$PLUGINS_DIR="/usr/local/bin/";}

if    (!$INSTALL_DIR && !$ROOT_INSTALL){$INSTALL_DIR="$HOME/bin/";mkpath ($INSTALL_DIR);}
elsif (!$INSTALL_DIR &&  $ROOT_INSTALL){$INSTALL_DIR="/usr/local/bin/";}

if (-d "mcoffee"){`cp mcoffee/* $TCM`;}


our $ENV_FILE="$TCDIR/t_coffee_env";
&env_file2putenv ($ENV_FILE);
&set_proxy($proxy);
my ($target, $p, $r);
$target=$p;

foreach $p (  ((keys (%PG)),(keys(%MODE)),(@smode)) )
  {
    if ($ARGV[0] eq $p && $target eq ""){$target=$p;}
  }
if ($target eq ""){exit ($EXIT_FAILURE);}


foreach $r (@required_applications)
  {
    my @app_list;
    my $i;
    $i=0;
    
    @app_list=split (/_OR_/, $r);
    foreach my $pg (@app_list)
      {
	$i+=&pg_is_installed ($pg);
      }
    if ($i==0)
      {
      print "One of the following packages must be installed to proceed: ";
      foreach my $pg (@app_list)
	{
	  print ("$pg ");
	}
      die;
    }
  }






&sign_license_ni();


$PG{C}{compiler}=get_C_compiler($CC);
$PG{Fortran}{compiler}=get_F_compiler($FC);
$PG{CXX}{compiler}=$PG{CPP}{compiler}=$PG{GPP}{compiler}=get_CXX_compiler($CXX);
if ($CXXFLAGS){$PG{CPP}{options}=$PG{GPP}{options}=$PG{CXX}{options}=$CXXFLAGS;}
if ($CFLAGS){$PG{C}{options}=$CFLAGS;}
foreach my $c (keys(%PG))
  {
    my $arguments;
    if ($PG{$c}{compiler})
      {
	$arguments="$PG{$c}{compiler_flag}=$PG{$c}{compiler} ";
	if ($PG{$c}{options})
	  {
	    $arguments.="$PG{$c}{options_flag}=$PG{$c}{options} ";
	  }
	$PG{$c}{arguments}=$arguments;
      }
  }

if ($PG{$target}){$PG{$target}{install}=1;}
else
  {
    foreach my $pg (keys(%PG))
      {
	if ( $target eq "all" || ($PG{$pg}{mode}=~/$target/))
	  {
	    $PG{$pg} {install}=1;
	  }
      }
  }

foreach my $pg (keys(%PG))
  {
    if (!$PG{$pg}{update_action}){$PG{$pg}{update_action}=$default_update_action;}
    elsif ($PG{$pg}{update_action} eq "never"){$PG{$pg}{install}=0;}
    if ( $force && $PG{$pg}{install})
      {
	`rm $BIN/$pg $BIN/$pg.exe $SILENT`;
      }
    if ($PG{$pg}{update_action} eq "update" && $PG{$pg}{install}){$PG{$pg}{update}=1;}
  }

if (($target=~/clean/))
  {
    print "------- cleaning executables -----\n";
    `rm bin/* $SILENT`;
    exit ($EXIT_SUCCESS);
  }

if ( !$PG{$target}){print "------- Installing T-Coffee Modes\n";}

foreach my $m (keys(%MODE))
  {
    if ( $target eq "all" || $target eq $m)
      {
	print "\n------- The installer will now install the $m components $MODE{$m}{description}\n";
	foreach my $pg (keys(%PG))
	  {
	    if ( $PG{$pg}{mode} =~/$m/ && $PG{$pg}{install})
	      {
		if ($PG{$pg}{touched}){print "------- $PG{$pg}{dname}: already processed\n";}
		else {$PG{$pg}{success}=&install_pg($pg);$PG{$pg}{touched}=1;}
	      }
	  }
      }
  }

if ( $PG{$target}){print "------- Installing Individual Package\n";}
foreach my $pg (keys (%PG))
  {
    
    if ( $PG{$pg}{install} && !$PG{$pg}{touched})
      {
	print "\n------- Install $pg\n";
	$PG{$pg}{success}=&install_pg($pg);$PG{$pg}{touched}=1;
      }
  }
print "------- Finishing The installation\n";
my $final_report=&install ($INSTALL_DIR);

print "\n";
print "*********************************************************************\n";
print "********              INSTALLATION SUMMARY          *****************\n";
print "*********************************************************************\n";
print "------- SUMMARY package Installation:\n";
foreach my $pg (keys(%PG))
  {
    if ( $PG{$pg}{install})
      {
	my $bin_status=($PG{$pg}{from_binary} && $PG{$pg}{success})?"[from binary]":"";
	if     ( $PG{$pg}{new} && !$PG{$pg}{old})                     {print "*------        $PG{$pg}{dname}: installed $bin_status\n"; $PG{$pg}{status}=1;}
	elsif  ( $PG{$pg}{new} &&  $PG{$pg}{old})                     {print "*------        $PG{$pg}{dname}: updated $bin_status\n"  ; $PG{$pg}{status}=1;} 
	elsif  (!$PG{$pg}{new} &&  $PG{$pg}{old} && !$PG{$pg}{update}){print "*------        $PG{$pg}{dname}: previous\n" ; $PG{$pg}{status}=1;}
	elsif  (!$PG{$pg}{new} &&  $PG{$pg}{old} &&  $PG{$pg}{update}){print "*------        $PG{$pg}{dname}: failed update (previous installation available)\n";$PG{$pg}{status}=0;}
	else                                                          {print "*------        $PG{$pg}{dname}: failed installation";$PG{$pg}{status}=0;}
      }
  }

if ( !$PG{$target}){print "*------ SUMMARY mode Installation:\n";}
foreach my $m (keys(%MODE))
  {
    if ( $target eq "all" || $target eq $m)
      {
	my $succesful=1;
	foreach my $pg (keys(%PG))
	  {
	    if (($PG{$pg}{mode}=~/$m/) && $PG{$pg}{install} && $PG{$pg}{status}==0)
	      {
		$succesful=0;
		print "*!!!!!!       $PG{$pg}{dname}: Missing\n";
	      }
	  }
	if ( $succesful)
	  {
	    $MODE{$m}{status}=1;
	    print "*------       MODE $MODE{$m}{dname} SUCCESFULY installed\n";
	  }
	else
	  {
	    $MODE{$m}{status}=0;
	    print "*!!!!!!       MODE $MODE{$m}{dname} UNSUCCESFULY installed\n";
	  }
      }
  }

if ($clean==1 && ($BASE=~/install4tcoffee/) ){print "*------ Clean Installation Directory: $BASE\n";`rm -rf $BASE`;}
foreach my $pg (keys(%PG)){if ($PG{$pg}{install} && $PG{$pg}{status}==0){exit ($EXIT_FAILURE);}}
exit ($EXIT_SUCCESS);  

sub get_CXX_compiler
  {
    my $c=@_[0];
    my (@clist)=("g++");
    
    return get_compil ($c, @clist);
 }
sub get_C_compiler
  {
    my $c=@_[0];
    my (@clist)=("gcc", "cc", "icc");
    
    return get_compil ($c, @clist);
 }

sub get_F_compiler
  {
    my ($c)=@_[0];
    my @clist=("f77", "g77", "gfortran", "ifort");
    return get_compil ($c, @clist);
  } 
       
sub get_compil
  {
    my ($fav,@clist)=(@_);
    
    #return the first compiler found installed in the system. Check first the favorite
    foreach my $c ($fav,@clist)
      {
	if  (&pg_is_installed ($c)){return $c;}
      }
    return "";
  }
sub exit_if_pg_not_installed
  {
    my (@arg)=(@_);
    
    foreach my $p (@arg)
      {
	if ( !&pg_is_installed ($p))
	  {
	    print "!!!!!!!! The $p utility must be installed for this installation to proceed [FATAL]\n";
	    die;
	  }
      }
    return 1;
  }
sub set_proxy
  {
    my ($proxy)=(@_);
    my (@list,$p);
    
    @list= ("HTTP_proxy", "http_proxy", "HTTP_PROXY", "ALL_proxy", "all_proxy","HTTP_proxy_4_TCOFFEE","http_proxy_4_TCOFFEE");
    
    if (!$proxy)
      {
	foreach my $p (@list)
	  {
	    if ( ($ENV_SET{$p}) || $ENV{$p}){$proxy=$ENV{$p};}
	  }
      }
    foreach my $p(@list){$ENV{$p}=$proxy;}
  }
	
sub check_internet_connection
  {
    my $internet;
    
    if ( -e "x"){unlink ("x");}
    if     (&pg_is_installed    ("wget")){`wget www.google.com -Ox >/dev/null 2>/dev/null`;}
    elsif  (&pg_is_installed    ("curl")){`curl www.google.com -ox >/dev/null 2>/dev/null`;}
    else
      {
	printf stderr "\nERROR: No pg for remote file fetching [wget or curl][FATAL]\n";
	exit ($EXIT_FAILURE);
      }
    
    if ( !-e "x" || -s "x" < 10){$internet=0;}
    else {$internet=1;}
    if (-e "x"){unlink "x";}
    return $internet;
  }
sub url2file
  {
    my ($cmd, $file,$wget_arg, $curl_arg)=(@_);
    my ($exit,$flag, $pg, $arg);
    
    if ($INTERNET || check_internet_connection ()){$INTERNET=1;}
    else
      {
	print STDERR "ERROR: No Internet Connection [FATAL:install.pl]\n";
	exit ($EXIT_FAILURE);
      }
    
    if     (&pg_is_installed    ("wget")){$pg="wget"; $flag="-O";$arg=$wget_arg;}
    elsif  (&pg_is_installed    ("curl")){$pg="curl"; $flag="-o";$arg=$curl_arg;}
    else
      {
	printf stderr "\nERROR: No pg for remote file fetching [wget or curl][FATAL]\n";
	exit ($EXIT_FAILURE);
      }
    
    
    if (-e $file){unlink($file);}
    $exit=system "$pg $cmd $flag$file $arg";
    return $exit;
  }

sub pg_is_installed
  {
    my ($p, $dir)=(@_);
    my ($r,$m);
    my ($supported, $language, $compil);
    
    if ( $PG{$p})
      {
	$language=$PG{$p}{language2};
	$compil=$PG{$language}{compiler};
      }
    
    if ( $compil eq "CPAN")
      {
	if ( system ("perl -M$p -e 1")==$EXIT_SUCCESS){return 1;}
	else {return 0;}
      }
    elsif ($dir)
      {
	if (-e "$dir/$p" || -e "$dir/$p\.exe"){return 1;}
	else {return 0;}
      }
    elsif (-e "$PLUGINS_DIR/$p" || -e "$PLUGINS_DIR/$p.exe"){return 1;}
    else
      {
	$r=`which $p 2>/dev/null`;
	if ($r eq ""){return 0;}
	else {return 1;}
      }
    return 0;
  }
sub install
  {
    my ($new_bin)=(@_);
    my ($copied, $report);

    
    if (!$ROOT_INSTALL)
      {
	
	if (-e "$BIN/t_coffee"){`$CP $BIN/t_coffee $INSTALL_DIR`};
	`cp $BIN/* $PLUGINS_DIR`;
	$copied=1;
      }
    else
      {
	$copied=&root_run ("You must be root to finalize the installation", "$CP $BIN/* $INSTALL_DIR $SILENT");
      }
    
     
  if ( !$copied)
    {
      $report="*!!!!!! Installation unsuccesful. The executables have been left in $BASE/bin\n";
    }
  elsif ( $copied && $ROOT)
    {
      $report="*------ Installation succesful. Your executables have been copied in $new_bin and are on your PATH\n";
    }
  elsif ( $copied && !$ROOT)
    {
      $report= "*!!!!!! T-Coffee and associated packages have been copied in: $new_bin\n";
      $report.="*!!!!!! This address is NOT in your PATH sytem variable\n";
      $report.="*!!!!!! You can do so by adding the following line in your ~/.bashrc file:\n";
      $report.="*!!!!!! export PATH=$new_bin:\$PATH\n";
    }
  return $report;
}

sub sign_license_ni
  {
    my $F=new FileHandle;
    open ($F, "license.txt");
    while (<$F>)
      {
	print "$_";
      }
    close ($F);
    
    return;
  }

sub install_pg
  {
    my ($pg)=(@_);
    my ($report, $previous, $language, $compiler, $return);
    
    if (!$PG{$pg}{install}){return 1;}
    
    $previous=&pg_is_installed ($pg);
    
    if ($PG{$pg}{update_action} eq "no_update" && $previous)
      {
	$PG{$pg}{old}=1;
	$PG{$pg}{new}=0;
	$return=1;
      }
    else
      {
	$PG{$pg}{old}=$previous;
	
	if ($PG{$pg} {language2} eq "Perl"){&install_perl_package ($pg);}
	elsif ($BINARIES_ONLY && &install_binary_package ($pg)){$PG{$pg}{from_binary}=1;}
	elsif (&install_source_package ($pg)){;}
	else 
	  {
	    
	    if (!&supported_os($OS))
	      {
		print "!!!!!!!! $pg compilation failed, binary unsupported for $OS\n"; 
	      }
	    elsif (!($PG{$pg}{from_binary}=&install_binary_package ($pg)))
	      {
		print "!!!!!!!! $pg compilation and  binary installation failed\n";
	      }
	  }
	$PG{$pg}{new}=$return=&pg_is_installed ($pg,$BIN);
      }

    
    return $return;
  }
sub install_perl_package
  {
    my ($pg)=(@_);
    my ($report, $language, $compiler);
    
    $language=$PG{$pg} {language2};
    $compiler=$PG{$language}{compiler};
    
    if (!&pg_is_installed ($pg))
      {
	if ( $OS eq "windows"){`perl -M$compiler -e 'install $pg'`;}
	elsif ( $ROOT eq "sudo"){system ("sudo perl -M$compiler -e 'install $pg'");}
	else {system ("su root -c perl -M$compiler -e 'install $pg'");}
      }
    return &pg_is_installed ($pg);
  }



sub install_source_package
  {
    my ($pg)=(@_);
    my ($report, $download, $arguments, $language, $address, $name, $ext, $main_dir, $distrib);
    my $wget_tmp="$TMP/wget.tmp";
    my (@fl);
    if ( -e "$BIN/$pg" || -e "$BIN/$pg.exe"){return 1;}
    
    if ($pg eq "t_coffee")  {return   &install_t_coffee ($pg);}
    elsif ($pg eq "TMalign"){return   &install_TMalign ($pg);}
    
    chdir $DISTRIBUTIONS;
    
    $download=$PG{$pg}{source};
    
    if (($download =~/tgz/))
      {
	($address,$name,$ext)=($download=~/(.+\/)([^\/]+)(\.tgz)/);
      }
    elsif (($download=~/tar\.gz/))
      {
	($address,$name,$ext)=($download=~/(.+\/)([^\/]+)(\.tar\.gz)/);
      }
    elsif (($download=~/tar/))
      {
	($address,$name,$ext)=($download=~/(.+\/)([^\/]+)(\.tar)/);
      }
    else
      {
	($address,$name)=($download=~/(.+\/)([^\/]+)/);
	$ext="";
      }
    $distrib="$name$ext";
    
    if ( !-d $pg){mkdir $pg;}
    chdir $pg;
   
    #get the distribution if available
    if ( -e "$DOWNLOAD_DIR/$distrib")
      {
	`$CP $DOWNLOAD_DIR/$distrib .`;
      }
    #UNTAR and Prepare everything
    if (!-e "$name.tar" && !-e "$name")
      {
	&check_rm ($wget_tmp);
	print "\n------- Downloading/Installing $pg\n";
	if (!-e $distrib && &url2file ("$download", "$wget_tmp")==$EXIT_SUCCESS)
	  {
	    
	    `mv $wget_tmp $distrib`;
	    `$CP $distrib $DOWNLOAD_DIR/`;
	  }

	if (!-e $distrib)
	  {
	    print "!!!!!!! Download of $pg distribution failed\n";
	    print "!!!!!!! Check Address: $PG{$pg}{source}\n";
	    return 0;
	  }
	print "\n------- unzipping/untaring $name\n";
	if (($ext =~/z/))
	  { 
	    &flush_command ("gunzip $name$ext");
	    
	  }
	if (($ext =~/tar/) || ($ext =~/tgz/))
	  {
	    &flush_command("tar -xvf $name.tar");
	  }
      }
    #Guess and enter the distribution directory
    @fl=ls($p);
    foreach my $f (@fl)
      {
	if (-d $f)
	  {
	    $main_dir=$f;
	  }
      }
    if (-d $main_dir)
	  {chdir $main_dir;}
    
    print "\n------- Compiling/Installing $pg\n";
    `make clean $SILENT`;
    #sap
    if ($pg eq "sap")
      {
	`rm *.o sap  sap.exe ./util/aa/*.o  ./util/wt/.o $SILENT`;
	&flush_command ("make $arguments sap");
	&check_cp ($pg, "$BIN");
      }
    elsif ($pg eq "clustalw2")
      {
	&flush_command("./configure");
	&flush_command("make $arguments");
	&check_cp ("./src/$pg", "$BIN");
	
      }
    elsif ($pg eq "clustalw")
      {
	&flush_command("make $arguments clustalw");
	`$CP $pg $BIN $SILENT`;
      }
    
    elsif ($pg eq "mafft")
      {
	my $base=cwd();
	my $c;
	
	#compile core
	mkpath ("./mafft/bin");
	mkpath ("./mafft/lib");
	chdir "$base/core";
	`make clean $SILENT`;
	&flush_command ("make $arguments");
	&flush_command ("make install LIBDIR=../mafft/lib BINDIR=../mafft/bin");
	
	#compile extension
	chdir "$base/extensions";
	`make clean $SILENT`;
	&flush_command ("make $arguments");
	&flush_command ("make install LIBDIR=../mafft/lib BINDIR=../mafft/bin");
	
	#put everything in mafft and copy the coompiled stuff in bin
	chdir "$base";
	if ($ROOT_INSTALL)
	  {
	    &root_run ("You Must be Roor to Install MAFFT\n", "mkdir /usr/local/mafft/;$CP mafft/lib/* /usr/local/mafft;$CP mafft/lib/mafft* /usr/local/bin ;$CP mafft/bin/mafft /usr/local/bin/; ");
	  }
	else
	  {
	    `$CP mafft/lib/*  $BIN`;
	    `$CP mafft/bin/mafft  $BIN`;
	  }
	`tar -cvf mafft.tar mafft`;
	`gzip mafft.tar`;
	`mv mafft.tar.gz $BIN`;
      }
    elsif ( $pg eq "dialign-tx")
      {
	my $f;
	my $base=cwd();

	chdir "./source";
	&flush_command (" make CPPFLAGS='-O3 -funroll-loops' all");
	
	chdir "..";
	&check_cp ("./source/$pg", "$BIN");
	&check_cp ("./source/$pg", "$BIN/dialign-t");
      }
    elsif ($pg eq "poa")
      {
	&flush_command ("make $arguments poa");
	&check_cp ("$pg", "$BIN");
      }
    elsif ( $pg eq "probcons")
      {
	`rm *.exe $SILENT`;
	&flush_command ("make $arguments probcons");
	&check_cp("$pg", "$BIN/$pg");
      }
    elsif ( $pg eq "probcons" || $pg eq "probconsRNA")
      {
	`rm *.exe $SILENT`;
	&flush_command ("make $arguments probcons");
	&check_cp("probcons", "$BIN/$pg");
      }

    elsif (  $pg eq "muscle")
      {
	`rm *.o muscle muscle.exe $SILENT`;
	&flush_command ("make $arguments all");
	&check_cp("$pg", "$BIN");
      }
    elsif ( $pg eq "pcma")
      {
	&flush_command ("make $arguments pcma");
	&check_cp("$pg", "$BIN");
      }
    elsif ($pg eq "kalign")
      {
	&flush_command ("./configure");
	&flush_command("make $arguments");
	&check_cp ("$pg",$BIN);
      }
    elsif ( $pg eq "amap")
      {
	chdir "align";
	`make clean $SILENT`;
	&flush_command ("make $arguments all");
	&check_cp ("$pg", $BIN);
      }
    elsif ( $pg eq "proda")
      {
	&flush_command ("make $arguments all");
	&check_cp ("$pg", $BIN);
      }
    elsif ( $pg eq "prank")
      {
	&flush_command ("make $arguments all");
	&check_cp ("$pg", $BIN);
      }
    elsif ( $pg eq "mustang")
      {
	&flush_command ("make $arguments all");
	if ( $OS=~/windows/){&check_cp("./bin/MUSTANG_v.3", "$BIN/mustang.exe");}
	else {&check_cp("./bin/MUSTANG_v.3", "$BIN/mustang");}
      }
    elsif ( $pg eq "RNAplfold")
      {
	&flush_command("./configure");
	&flush_command ("make $arguments all");
	&check_cp("./Progs/RNAplfold", "$BIN");
      }
    chdir $CDIR;
    return &pg_is_installed ($pg, $BIN);
  }

sub install_t_coffee
  {
    my ($pg)=(@_);
    my ($report,$cflags, $arguments, $language, $compiler) ;
    #1-Install T-Coffee
    chdir "t_coffee_source";
    &flush_command ("make clean");
    print "\n------- Compiling T-Coffee\n";
    $language=$PG{$pg} {language2};
    $arguments=$PG{$language}{arguments};
    if (!($arguments =~/CFLAGS/)){$arguments .= " CFLAGS=-O2 ";}

    if ( $CC ne ""){&flush_command ("make -i $arguments t_coffee");}
    &check_cp ($pg, $BIN);
    
    chdir $CDIR;
    return &pg_is_installed ($pg, $BIN);
  }
sub install_TMalign
  {
    my ($pg)=(@_);
    my $report;
    chdir "t_coffee_source";
    print "\n------- Compiling TMalign\n";
    `rm TMalign TMalign.exe $SILENT`;
    if ( $FC ne ""){&flush_command ("make -i $PG{Fortran}{arguments} TMalign");}
    &check_cp ($pg, $BIN);
    if ( !-e "$BIN/$pg" && pg_has_binary_distrib ($pg))
      {
	print "!!!!!!! Compilation of $pg impossible. Will try to install from binary\n";
	return &install_binary_package ($pg);
      }
    chdir $CDIR;
    return &pg_is_installed ($pg, $BIN);
  }

sub pg_has_binary_distrib
  {
    my ($pg)=(@_);
    if ($PG{$pg}{windows}){return 1;}
    elsif ($PG{$pg}{osx}){return 1;}
    elsif ($PG{$pg}{linux}){return 1;}
    return 0;
  }
sub install_binary_package
  {
    my ($pg)=(@_);
    my ($base,$report,$name, $download, $arguments, $language, $dir);
    my $isdir;
    &input_os();
    
    if (!&supported_os($OS)){return 0;}
    if ( $PG{$pg}{binary}){$name=$PG{$pg}{binary};}
    else 
      {
	$name=$pg;
	if ( $OS eq "windows"){$name.=".exe";}
      }
    
    $download="$WEB_BASE/Packages/Binaries/$OS/$name";
    
    $base=cwd();
    chdir $TMP;
    
    if (!-e $name)
      {
	`rm x $SILENT`;
	if ( url2file("$download","x")==$EXIT_SUCCESS)
	  {
	    `mv x $name`;
	  }
      }
    
    if (!-e $name)
      {
	print "!!!!!!! $PG{$pg}{dname}: Download of $pg binary failed\n";
	print "!!!!!!! $PG{$pg}{dname}: Check Address: $download\n";
	return 0;
      }
    print "\n------- Installing $pg\n";
    
    if ($name =~/tar\.gz/)
      {
	`gunzip  $name`;
	`tar -xvf $pg.tar`;
	chdir $pg;
	if ( $pg eq "mafft")
	  {
	    if ($ROOT_INSTALL)
	      {
		&root_run ("You Must be Roor to Install MAFFT\n", "$CP mafft/bin/* /usr/local/mafft;mkdir /usr/local/mafft/; $CP mafft/lib/* /usr/local/bin/");
	      }
	    else
	      {
		`$CP $TMP/$pg/bin/* $BIN $SILENT`;
		`$CP $TMP/$pg/lib/* $BIN $SILENT`;
	      }
	  }
	else
	  {
	    if (-e "$TMP/$pg/data"){`$CP $TMP/$pg/data/* $TCM $SILENT`;}
	    if (!($pg=~/\*/)){`rm -rf $pg`;}
	  }
      }
    else
      {
	&check_cp ("$pg", "$BIN");
	`chmod u+x $BIN/$pg`; 
	unlink ($pg);
      }
    chdir $base;
    $PG{$pg}{from_binary}=1;
    return &pg_is_installed ($pg, $BIN);
  }

sub add_dir 
  {
    my $dir=@_[0];
    
    if (!-e $dir && !-d $dir)
      {
	return mkpath ($dir);
      }
    else
      {
	return 0;
      }
  }
sub check_rm 
  {
    my ($file)=(@_);
    
    if ( -e $file)
      {
	return unlink($file);
      }
    return 0;
  }
sub check_cp
  {
    my ($from, $to)=(@_);
    if ( !-e $from && -e "$from\.exe"){$from="$from\.exe";}
    if ( !-e $from){return 0;}
        
    `$CP $from $to`;
    return 1;
  }
sub check_file_list_exists 
  {
    my ($base, @flist)=(@_);
    my $f;

    foreach $f (@flist)
      {
	if ( !-e "$base/$f"){return 0;}
      }
    return 1;
  }
sub ls
  {
    my $f=@_[0];
    my @fl;
    chomp(@fl=`ls -1 $f`);
    return @fl;
  }
sub flush_command
  {
    my $command=@_[0];
    my $F=new FileHandle;
    open ($F, "$command|");
    while (<$F>){print "    --- $_";}
    close ($F);
  }    

sub input_installation_directory
  {
    my $dir=@_[0];
    my $new;
    
    print "------- The current installation directory is: [$dir]\n";
    print "??????? Return to keep the default or new value:";
   
    if ($NO_QUESTION==0)
      {
	chomp ($new=<stdin>);
	while ( $new ne "" && !input_yes ("You have entered $new. Is this correct? ([y]/n):"))
	  {
	    print "???????New installation directory:";
	    chomp ($new=<stdin>);
	  }
	$dir=($new eq "")?$dir:$new;
	$dir=~s/\/$//;
      }
    
    if ( -d $dir){return $dir;}
    elsif (&root_run ("You must be root to create $dir","mkdir $dir")==$EXIT_SUCCESS){return $dir;}
    else
      {
	print "!!!!!!! $dir could not be created\n";
	if ( $NO_QUESTION)
	  {
	    return "";
	  }
	elsif ( &input_yes ("??????? Do you want to provide a new directory([y]/n)?:"))
	  {
	    return input_installation_directory ($dir);
	  }
	else
	  {
	    return "";
	  }
      }
    
  }
sub input_yes
  {
    my $question =@_[0];
    my $answer;

    if ($NO_QUESTION==1){return 1;}
    
    if ($question eq ""){$question="??????? Do you wish to proceed ([y]/n)?:";}
    print $question;
    chomp($answer=lc(<STDIN>));
    if (($answer=~/^y/) || $answer eq ""){return 1;}
    elsif ( ($answer=~/^n/)){return 0;}
    else
      {
	return input_yes($question);
      }
  }
sub root_run
  {
    my ($txt, $cmd)=(@_);
    
    if ( system ($cmd)==$EXIT_SUCCESS){return $EXIT_SUCCESS;}
    else 
      {
	print "------- $txt\n";
	if ( $ROOT eq "sudo"){return system ("sudo $cmd");}
	else {return system ("su root -c \"$cmd\"");}
      }
  }
sub get_root
  {
    if (&pg_is_installed ("sudo")){return "sudo";}
    else {return "su";}
  }

sub get_os
  {
    my $raw_os=`uname`;
    my $os;

    $raw_os=lc ($raw_os);
    
    if ($raw_os =~/cygwin/){$os="windows";}
    elsif ($raw_os =~/linux/){$os="linux";}
    elsif ($raw_os =~/osx/){$os="macosx";}
    elsif ($raw_os =~/darwin/){$os="macosx";}
    else
      {
	$os=$raw_os;
      }
    return $os;
  }
sub input_os
  {
    my $answer;
    if ($OS) {return $OS;}
    
    print "??????? which os do you use: [w]indows, [l]inux, [m]acosx:?";
    $answer=lc(<STDIN>);

    if (($answer=~/^m/)){$OS="macosx";}
    elsif ( ($answer=~/^w/)){$OS="windows";}
    elsif ( ($answer=~/^linux/)){$OS="linux";}
    
    else
      {
	return &input_os();
      }
    return $OS;
  }

sub supported_os
  {
    my ($os)=(@_[0]);
    return $SUPPORTED_OS{$os};
  }
    
    


sub update_tclinkdb 
  {
    my $file =@_[0];
    my $name;
    my $F=new FileHandle;
    my ($download, $address, $name, $l, $db);
    
    if ( $file eq "update"){$file=$TCLINKDB_ADDRESS;}
    
    if ( $file =~/http:\/\// || $file =~/ftp:\/\//)
      {
	($address, $name)=($download=~/(.*)\/([^\/]+)$/);
	`rm x $SILENT`;
	if (&url2file ($file,"x")==$EXIT_SUCCESS)
	  {
	    print "------- Susscessful upload of $name";
	    `mv x $name`;
	    $file=$name;
	  }
      }
    open ($F, "$file");
    while (<$F>)
      {
	my $l=$_;
	if (($l =~/^\/\//) || ($db=~/^#/)){;}
	elsif ( !($l =~/\w/)){;}
	else
	  {
	    my @v=split (/\s+/, $l);
	    if ( $l=~/^MODE/)
	      {
		$MODE{$v[1]}{$v[2]}=$v[3];
	      }
	    elsif ($l=~/^PG/)
	      {
		$PG{$v[1]}{$v[2]}=$v[3];
	      }
	  }
      }
    close ($F);
    &post_process_PG();
    return;
  }



sub initialize_PG
  {
    
$PG{"t_coffee"}{"4_TCOFFEE"}="TCOFFEE";
$PG{"t_coffee"}{"type"}="sequence_multiple_aligner";
$PG{"t_coffee"}{"ADDRESS"}="http://www.tcoffee.org";
$PG{"t_coffee"}{"language"}="C";
$PG{"t_coffee"}{"language2"}="C";
$PG{"t_coffee"}{"source"}="http://www.tcoffee.org/Packages/T-COFFEE_distribution.tar.gz";
$PG{"t_coffee"}{"update_action"}="always";
$PG{"t_coffee"}{"mode"}="tcoffee,mcoffee,rcoffee,expresso,3dcoffee";
$PG{"clustalw2"}{"4_TCOFFEE"}="CLUSTALW2";
$PG{"clustalw2"}{"type"}="sequence_multiple_aligner";
$PG{"clustalw2"}{"ADDRESS"}="http://www.clustal.org";
$PG{"clustalw2"}{"language"}="C++";
$PG{"clustalw2"}{"language2"}="CXX";
$PG{"clustalw2"}{"source"}="http://www.clustal.org/download/2.0.10/clustalw-2.0.10-src.tar.gz";
$PG{"clustalw2"}{"mode"}="mcoffee,rcoffee";
$PG{"clustalw"}{"4_TCOFFEE"}="CLUSTALW";
$PG{"clustalw"}{"type"}="sequence_multiple_aligner";
$PG{"clustalw"}{"ADDRESS"}="http://www.clustal.org";
$PG{"clustalw"}{"language"}="C";
$PG{"clustalw"}{"language2"}="C";
$PG{"clustalw"}{"source"}="http://www.clustal.org/download/1.X/ftp-igbmc.u-strasbg.fr/pub/ClustalW/clustalw1.82.UNIX.tar.gz";
$PG{"clustalw"}{"mode"}="mcoffee,rcoffee";
$PG{"dialign-t"}{"4_TCOFFEE"}="DIALIGNT";
$PG{"dialign-t"}{"type"}="sequence_multiple_aligner";
$PG{"dialign-t"}{"ADDRESS"}="http://dialign-tx.gobics.de/";
$PG{"dialign-t"}{"DIR"}="/usr/share/dialign-tx/";
$PG{"dialign-t"}{"language"}="C";
$PG{"dialign-t"}{"language2"}="C";
$PG{"dialign-t"}{"source"}="http://dialign-tx.gobics.de/DIALIGN-TX_1.0.1.tar.gz";
$PG{"dialign-t"}{"mode"}="mcoffee";
$PG{"dialign-t"}{"binary"}="dialign-t";
$PG{"dialign-tx"}{"4_TCOFFEE"}="DIALIGNTX";
$PG{"dialign-tx"}{"type"}="sequence_multiple_aligner";
$PG{"dialign-tx"}{"ADDRESS"}="http://dialign-tx.gobics.de/";
$PG{"dialign-tx"}{"DIR"}="/usr/share/dialign-tx/";
$PG{"dialign-tx"}{"language"}="C";
$PG{"dialign-tx"}{"language2"}="C";
$PG{"dialign-tx"}{"source"}="http://dialign-tx.gobics.de/DIALIGN-TX_1.0.1.tar.gz";
$PG{"dialign-tx"}{"mode"}="mcoffee";
$PG{"dialign-tx"}{"binary"}="dialign-tx";
$PG{"poa"}{"4_TCOFFEE"}="POA";
$PG{"poa"}{"type"}="sequence_multiple_aligner";
$PG{"poa"}{"ADDRESS"}="http://www.bioinformatics.ucla.edu/poa/";
$PG{"poa"}{"language"}="C";
$PG{"poa"}{"language2"}="C";
$PG{"poa"}{"source"}="http://downloads.sourceforge.net/poamsa/poaV2.tar.gz";
$PG{"poa"}{"DIR"}="/usr/share/";
$PG{"poa"}{"FILE1"}="blosum80.mat";
$PG{"poa"}{"mode"}="mcoffee";
$PG{"poa"}{"binary"}="poa";
$PG{"probcons"}{"4_TCOFFEE"}="PROBCONS";
$PG{"probcons"}{"type"}="sequence_multiple_aligner";
$PG{"probcons"}{"ADDRESS"}="http://probcons.stanford.edu/";
$PG{"probcons"}{"language2"}="CXX";
$PG{"probcons"}{"language"}="C++";
$PG{"probcons"}{"source"}="http://probcons.stanford.edu/probcons_v1_12.tar.gz";
$PG{"probcons"}{"mode"}="mcoffee";
$PG{"probcons"}{"binary"}="probcons";
$PG{"mafft"}{"4_TCOFFEE"}="MAFFT";
$PG{"mafft"}{"type"}="sequence_multiple_aligner";
$PG{"mafft"}{"ADDRESS"}="http://align.bmr.kyushu-u.ac.jp/mafft/online/server/";
$PG{"mafft"}{"language"}="C";
$PG{"mafft"}{"language"}="C";
$PG{"mafft"}{"source"}="http://align.bmr.kyushu-u.ac.jp/mafft/software/mafft-6.603-with-extensions-src.tgz";
$PG{"mafft"}{"windows"}="http://align.bmr.kyushu-u.ac.jp/mafft/software/mafft-6.603-mingw.tar";
$PG{"mafft"}{"mode"}="mcoffee,rcoffee";
$PG{"mafft"}{"binary"}="mafft.tar.gz";
$PG{"muscle"}{"4_TCOFFEE"}="MUSCLE";
$PG{"muscle"}{"type"}="sequence_multiple_aligner";
$PG{"muscle"}{"ADDRESS"}="http://www.drive5.com/muscle/";
$PG{"muscle"}{"language"}="C++";
$PG{"muscle"}{"language2"}="GPP";
$PG{"muscle"}{"source"}="http://www.drive5.com/muscle/downloads3.6/muscle3.6_src.tar.gz";
$PG{"muscle"}{"windows"}="http://www.drive5.com/muscle/downloads3.6/muscle3.6_win32.zip";
$PG{"muscle"}{"linux"}="http://www.drive5.com/muscle/downloads3.6/muscle3.6_linux_ia32.tar.gz";
$PG{"muscle"}{"mode"}="mcoffee,rcoffee";
$PG{"pcma"}{"4_TCOFFEE"}="PCMA";
$PG{"pcma"}{"type"}="sequence_multiple_aligner";
$PG{"pcma"}{"ADDRESS"}="ftp://iole.swmed.edu/pub/PCMA/";
$PG{"pcma"}{"language"}="C";
$PG{"pcma"}{"language2"}="C";
$PG{"pcma"}{"source"}="ftp://iole.swmed.edu/pub/PCMA/pcma.tar.gz";
$PG{"pcma"}{"mode"}="mcoffee";
$PG{"kalign"}{"4_TCOFFEE"}="KALIGN";
$PG{"kalign"}{"type"}="sequence_multiple_aligner";
$PG{"kalign"}{"ADDRESS"}="http://msa.cgb.ki.se";
$PG{"kalign"}{"language"}="C";
$PG{"kalign"}{"language2"}="C";
$PG{"kalign"}{"source"}="http://msa.cgb.ki.se/downloads/kalign/current.tar.gz";
$PG{"kalign"}{"mode"}="mcoffee";
$PG{"amap"}{"4_TCOFFEE"}="AMAP";
$PG{"amap"}{"type"}="sequence_multiple_aligner";
$PG{"amap"}{"ADDRESS"}="http://bio.math.berkeley.edu/amap/";
$PG{"amap"}{"language"}="C++";
$PG{"amap"}{"language2"}="CXX";
$PG{"amap"}{"source"}="http://baboon.math.berkeley.edu/amap/download/amap.2.2.tar.gz";
$PG{"amap"}{"mode"}="mcoffee";
$PG{"proda"}{"4_TCOFFEE"}="PRODA";
$PG{"proda"}{"type"}="sequence_multiple_aligner";
$PG{"proda"}{"ADDRESS"}="http://proda.stanford.edu";
$PG{"proda"}{"language"}="C++";
$PG{"proda"}{"language2"}="CXX";
$PG{"proda"}{"source"}="http://proda.stanford.edu/proda_1_0.tar.gz";
$PG{"proda"}{"mode"}="mcoffee";
$PG{"prank"}{"4_TCOFFEE"}="PRANK";
$PG{"prank"}{"type"}="sequence_multiple_aligner";
$PG{"prank"}{"ADDRESS"}="http://www.ebi.ac.uk/goldman-srv/prank/";
$PG{"prank"}{"language"}="C++";
$PG{"prank"}{"language2"}="CXX";
$PG{"prank"}{"source"}="http://www.ebi.ac.uk/goldman-srv/prank/src/old/prank.src.081202.tgz";
$PG{"prank"}{"mode"}="mcoffee";
$PG{"sap"}{"4_TCOFFEE"}="SAP";
$PG{"sap"}{"type"}="structure_pairwise_aligner";
$PG{"sap"}{"ADDRESS"}="http://mathbio.nimr.mrc.ac.uk/wiki/Software";
$PG{"sap"}{"language"}="C";
$PG{"sap"}{"language2"}="C";
$PG{"sap"}{"source"}="http://www.tcoffee.org/Packages/sap_distribution_TCC_0.6.tar.gz";
$PG{"sap"}{"mode"}="expresso,3dcoffee";
$PG{"TMalign"}{"4_TCOFFEE"}="TMALIGN";
$PG{"TMalign"}{"type"}="structure_pairwise_aligner";
$PG{"TMalign"}{"ADDRESS"}="http://zhang.bioinformatics.ku.edu/TM-align/TMalign.f";
$PG{"TMalign"}{"language"}="Fortran";
$PG{"TMalign"}{"language2"}="Fortran";
$PG{"TMalign"}{"source"}="http://zhang.bioinformatics.ku.edu/TM-align/TMalign.f";
$PG{"TMalign"}{"linux"}="http://zhang.bioinformatics.ku.edu/TM-align/TMalign_32.gz";
$PG{"TMalign"}{"mode"}="expresso,3dcoffee";
$PG{"mustang"}{"4_TCOFFEE"}="MUSTANG";
$PG{"mustang"}{"type"}="structure_pairwise_aligner";
$PG{"mustang"}{"ADDRESS"}="http://www.cs.mu.oz.au/~arun/mustang";
$PG{"mustang"}{"language"}="C++";
$PG{"mustang"}{"language2"}="CXX";
$PG{"mustang"}{"source"}="http://www.cs.mu.oz.au/~arun/mustang/mustang_v.3.tgz";
$PG{"mustang"}{"mode"}="expresso,3dcoffee";
$PG{"lsqman"}{"4_TCOFFEE"}="LSQMAN";
$PG{"lsqman"}{"type"}="structure_pairwise_aligner";
$PG{"lsqman"}{"ADDRESS"}="empty";
$PG{"lsqman"}{"language"}="empty";
$PG{"lsqman"}{"language2"}="empty";
$PG{"lsqman"}{"source"}="empty";
$PG{"lsqman"}{"update_action"}="never";
$PG{"lsqman"}{"mode"}="expresso,3dcoffee";
$PG{"align_pdb"}{"4_TCOFFEE"}="ALIGN_PDB";
$PG{"align_pdb"}{"type"}="structure_pairwise_aligner";
$PG{"align_pdb"}{"ADDRESS"}="empty";
$PG{"align_pdb"}{"language"}="empty";
$PG{"align_pdb"}{"language2"}="empty";
$PG{"align_pdb"}{"source"}="empty";
$PG{"align_pdb"}{"update_action"}="never";
$PG{"align_pdb"}{"mode"}="expresso,3dcoffee";
$PG{"fugueali"}{"4_TCOFFEE"}="FUGUE";
$PG{"fugueali"}{"type"}="structure_pairwise_aligner";
$PG{"fugueali"}{"ADDRESS"}="http://www-cryst.bioc.cam.ac.uk/fugue/download.html";
$PG{"fugueali"}{"language"}="empty";
$PG{"fugueali"}{"language2"}="empty";
$PG{"fugueali"}{"source"}="empty";
$PG{"fugueali"}{"update_action"}="never";
$PG{"fugueali"}{"mode"}="expresso,3dcoffee";
$PG{"dalilite.pl"}{"4_TCOFFEE"}="DALILITEc";
$PG{"dalilite.pl"}{"type"}="structure_pairwise_aligner";
$PG{"dalilite.pl"}{"ADDRESS"}="built_in";
$PG{"dalilite.pl"}{"ADDRESS2"}="http://www.ebi.ac.uk/Tools/webservices/services/dalilite";
$PG{"dalilite.pl"}{"language"}="Perl";
$PG{"dalilite.pl"}{"language2"}="Perl";
$PG{"dalilite.pl"}{"source"}="empty";
$PG{"dalilite.pl"}{"update_action"}="never";
$PG{"dalilite.pl"}{"mode"}="expresso,3dcoffee";
$PG{"probconsRNA"}{"4_TCOFFEE"}="PROBCONSRNA";
$PG{"probconsRNA"}{"type"}="RNA_multiple_aligner";
$PG{"probconsRNA"}{"ADDRESS"}="http://probcons.stanford.edu/";
$PG{"probconsRNA"}{"language"}="C++";
$PG{"probconsRNA"}{"language2"}="CXX";
$PG{"probconsRNA"}{"source"}="http://probcons.stanford.edu/probconsRNA.tar.gz";
$PG{"probconsRNA"}{"mode"}="mcoffee,rcoffee";
$PG{"sfold"}{"4_TCOFFEE"}="CONSAN";
$PG{"sfold"}{"type"}="RNA_pairwise_aligner";
$PG{"sfold"}{"ADDRESS"}="http://selab.janelia.org/software/consan/";
$PG{"sfold"}{"language"}="empty";
$PG{"sfold"}{"language2"}="empty";
$PG{"sfold"}{"source"}="empty";
$PG{"sfold"}{"update_action"}="never";
$PG{"sfold"}{"mode"}="rcoffee";
$PG{"RNAplfold"}{"4_TCOFFEE"}="RNAPLFOLD";
$PG{"RNAplfold"}{"type"}="RNA_secondarystructure_predictor";
$PG{"RNAplfold"}{"ADDRESS"}="http://www.tbi.univie.ac.at/~ivo/RNA/";
$PG{"RNAplfold"}{"language"}="C";
$PG{"RNAplfold"}{"language2"}="C";
$PG{"RNAplfold"}{"source"}="http://www.tbi.univie.ac.at/~ivo/RNA/ViennaRNA-1.7.2.tar.gz";
$PG{"RNAplfold"}{"mode"}="rcoffee";
$PG{"hmmtop"}{"4_TCOFFEE"}="HMMTOP";
$PG{"hmmtop"}{"type"}="protein_secondarystructure_predictor";
$PG{"hmmtop"}{"ADDRESS"}="www.enzim.hu/hmmtop/";
$PG{"hmmtop"}{"language"}="C";
$PG{"hmmtop"}{"language2"}="C";
$PG{"hmmtop"}{"source"}="empty";
$PG{"hmmtop"}{"update_action"}="never";
$PG{"hmmtop"}{"mode"}="tcoffee";
$PG{"gorIV"}{"4_TCOFFEE"}="GOR4";
$PG{"gorIV"}{"type"}="protein_secondarystructure_predictor";
$PG{"gorIV"}{"ADDRESS"}="http://mig.jouy.inra.fr/logiciels/gorIV/";
$PG{"gorIV"}{"language"}="C";
$PG{"gorIV"}{"language2"}="C";
$PG{"gorIV"}{"source"}="http://mig.jouy.inra.fr/logiciels/gorIV/GOR_IV.tar.gz";
$PG{"gorIV"}{"update_action"}="never";
$PG{"gorIV"}{"mode"}="tcoffee";
$PG{"wublast.pl"}{"4_TCOFFEE"}="EBIWUBLASTc";
$PG{"wublast.pl"}{"type"}="protein_homology_predictor";
$PG{"wublast.pl"}{"ADDRESS"}="built_in";
$PG{"wublast.pl"}{"ADDRESS2"}="http://www.ebi.ac.uk/Tools/webservices/services/wublast";
$PG{"wublast.pl"}{"language"}="Perl";
$PG{"wublast.pl"}{"language2"}="Perl";
$PG{"wublast.pl"}{"source"}="empty";
$PG{"wublast.pl"}{"update_action"}="never";
$PG{"wublast.pl"}{"mode"}="psicoffee,expresso,3dcoffee";
$PG{"blastpgp.pl"}{"4_TCOFFEE"}="EBIBLASTPGPc";
$PG{"blastpgp.pl"}{"type"}="protein_homology_predictor";
$PG{"blastpgp.pl"}{"ADDRESS"}="built_in";
$PG{"blastpgp.pl"}{"ADDRESS2"}="http://www.ebi.ac.uk/Tools/webservices/services/blastpgp";
$PG{"blastpgp.pl"}{"language"}="Perl";
$PG{"blastpgp.pl"}{"language2"}="Perl";
$PG{"blastpgp.pl"}{"source"}="empty";
$PG{"blastpgp.pl"}{"update_action"}="never";
$PG{"blastpgp.pl"}{"mode"}="psicoffee,expresso,3dcoffee";
$PG{"blastcl3"}{"4_TCOFFEE"}="NCBIWEBBLAST";
$PG{"blastcl3"}{"type"}="protein_homology_predictor";
$PG{"blastcl3"}{"ADDRESS"}="ftp://ftp.ncbi.nih.gov/blast/executables/LATEST";
$PG{"blastcl3"}{"language"}="C";
$PG{"blastcl3"}{"language2"}="C";
$PG{"blastcl3"}{"source"}="empty";
$PG{"blastcl3"}{"update_action"}="never";
$PG{"blastcl3"}{"mode"}="psicoffee,expresso,3dcoffee";
$PG{"blastpgp"}{"4_TCOFFEE"}="NCBIBLAST";
$PG{"blastpgp"}{"type"}="protein_homology_predictor";
$PG{"blastpgp"}{"ADDRESS"}="ftp://ftp.ncbi.nih.gov/blast/executables/LATEST";
$PG{"blastpgp"}{"language"}="C";
$PG{"blastpgp"}{"language2"}="C";
$PG{"blastpgp"}{"source"}="empty";
$PG{"blastpgp"}{"update_action"}="never";
$PG{"blastpgp"}{"mode"}="psicoffee,expresso,3dcoffee";
$PG{"SOAP::Lite"}{"4_TCOFFEE"}="SOAPLITE";
$PG{"SOAP::Lite"}{"type"}="library";
$PG{"SOAP::Lite"}{"ADDRESS"}="http://cpansearch.perl.org/src/MKUTTER/SOAP-Lite-0.710.08/Makefile.PL";
$PG{"SOAP::Lite"}{"language"}="Perl";
$PG{"SOAP::Lite"}{"language2"}="Perl";
$PG{"SOAP::Lite"}{"source"}="empty";
$PG{"SOAP::Lite"}{"mode"}="psicoffee,expresso,3dcoffee";
$MODE{"tcoffee"}{"name"}="tcoffee";
$MODE{"rcoffee"}{"name"}="rcoffee";
$MODE{"3dcoffee"}{"name"}="3dcoffee";
$MODE{"mcoffee"}{"name"}="mcoffee";
$MODE{"expresso"}{"name"}="expresso";


$PG{C}{compiler}="gcc";
$PG{C}{compiler_flag}="CC";
$PG{C}{options}="";
$PG{C}{options_flag}="CFLAGS";
$PG{C}{type}="compiler";

$PG{"CXX"}{compiler}="g++";
$PG{"CXX"}{compiler_flag}="CXX";
$PG{"CXX"}{options}="";
$PG{"CXX"}{options_flag}="CXXFLAGS";
$PG{CXX}{type}="compiler";

$PG{"CPP"}{compiler}="g++";
$PG{"CPP"}{compiler_flag}="CPP";
$PG{"CPP"}{options}="";
$PG{"CPP"}{options_flag}="CPPFLAGS";
$PG{CPP}{type}="compiler";

$PG{"GPP"}{compiler}="g++";
$PG{"GPP"}{compiler_flag}="GPP";
$PG{"GPP"}{options}="";
$PG{"GPP"}{options_flag}="CFLAGS";
$PG{GPP}{type}="compiler";

$PG{Fortran}{compiler}="g77";
$PG{Fortran}{compiler_flag}="FCC";
$PG{Fortran}{type}="compiler";

$PG{Perl}{compiler}="CPAN";
$PG{Perl}{type}="compiler";

$SUPPORTED_OS{macox}="Macintosh";
$SUPPORTED_OS{linux}="Linux";
$SUPPORTED_OS{windows}="Cygwin";



$MODE{t_coffee}{description}=" for regular multiple sequence alignments";
$MODE{rcoffee} {description}=" for RNA multiple sequence alignments";

$MODE{psicoffee} {description}=" for Homology Extended multiple sequence alignments";
$MODE{expresso}{description}=" for very accurate structure based multiple sequence alignments";
$MODE{"3dcoffee"}{description}=" for multiple structure alignments";
$MODE{mcoffee} {description}=" for combining alternative multiple sequence alignment packages\n------- into a unique meta-package. The installer will upload several MSA packages and compile them\n
";


&post_process_PG();
return;
}

sub post_process_PG
  {
    my $p;
    
    %PG=&name2dname (%PG);
    %MODE=&name2dname(%MODE);
    foreach $p (keys(%PG)){if ( $PG{$p}{type} eq "compiler"){$PG{$p}{update_action}="never";}}
    
  }

sub name2dname
  {
    my (%L)=(@_);
    my ($l, $ml);
    
    foreach my $pg (keys(%L))
      {
	$l=length ($pg);
	if ( $l>$ml){$ml=$l;}
      }
    $ml+=1;
    foreach my $pg (keys(%L))
      {
	my $name;
	$l=$ml-length ($pg);
	$name=$pg;
	for ( $b=0; $b<$l; $b++)
	  {
	    $name .=" ";
	  }
	$L{$pg}{dname}=$name;
      }
    return %L;
  }

sub env_file2putenv
  {
    my $f=@_[0];
    my $F=new FileHandle;
    my $n;
    
    open ($F, "$f");
    while (<$F>)
      {
	my $line=$_;
	my($var, $value)=($_=~/(\S+)\=(\S*)/);
	$ENV{$var}=$value;
	$ENV_SET{$var}=1;
	$n++;
      }
    close ($F);
    return $n;
  }

