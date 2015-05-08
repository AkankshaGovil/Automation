##
## Perl Readline -- The Quick Help
## (see the manual for complete info)
##
## Once this package is included (require'd), you can then call
##	$text = &readline'readline($input);
## to get lines of input from the user.
##
## Normally, it reads ~/.inputrc when loaded... to suppress this, set
## 	$readline'rl_NoInitFromFile = 1;
## before requiring the package.
##
## Call rl_bind to add your own key bindings, as in
##	&readline'rl_bind('C-L', 'possible-completions');
##
## Call rl_set to set mode variables yourself, as in
##	&readline'rl_set('TcshCompleteMode', 'On');
##
## If $ENV{EDITOR} is a string containing the substring 'vi', we start in vi
## input mode; otherwise start in emacs mode.  To override this behavior, do
## 	   &readline::rl_set('EditingMode', 'vi');
## 	or &readline::rl_set('EditingMode', 'emacs');
##
## Call rl_basic_commands to set your own command completion, as in
##      &readline'rl_basic_commands('print', 'list', 'quit', 'run', 'status');
##
##

package readline;

my $autoload_broken = 1;	# currently: defined does not work with a-l
my $useioctl = 1;

##
## BLURB:
## A pretty full-function package similar to GNU's readline.
## Includes support for EUC-encoded Japanese text.
##
## Written by Jeffrey Friedl, Omron Corporation (jfriedl@omron.co.jp)
##
## Comments, corrections welcome.
##
## Thanks to the people at FSF for readline (and the code I referenced
## while writing this), and for Roland Schemers whose line_edit.pl I used
## as an early basis for this.
##
$VERSION = $VERSION = '0.9908';

# 1000510.010 - Changes from Joe Petolino (petolino@eng.sun.com), requested
##              by Ilya:
##
##              * Make it compatible with perl 5.003.
##              * Rename getc() to getc_with_pending().
##              * Change unshift(@Pending) to push(@Pending).
##
## 991109.009 - Changes from Joe Petolino (petolino@eng.sun.com):
##              Added vi mode.  Also added a way to set the keymap default
##      	action for multi-character keymaps, so that a 2-character
##      	sequence (e.g. <esc>A) can be treated as two one-character
##      	commands (<esc>, then A) if the sequence is not explicitly
##              mapped.
##      
##              Changed subs:
##
##              * preinit(): Initialize new keymaps and other data structures.
##		             Use $ENV{EDITOR} to set startup mode.
##
##              * init():    Sets the global *KeyMap, since &F_ReReadInitFile
##                           may have changed the key map.
##
##		* InitKeymap(): $KeyMap{default} is now optional - don't
##			     set it if $_[1] eq '';
##
##		* actually_do_binding(): Set $KeyMap{default} for '\*' key;
##			     warning if double-defined.
##
##		* rl_bind(): Implement \* to set the keymap default.  Also fix
##			     some existing regex bugs that I happened to notice.
##
##		* readline(): No longer takes input from $pending before
##                           calling &$rl_getc(); instead, it calls getc_with_pending(),
##                           which takes input from the new array @Pending
##			     before calling &$rl_getc().  Sets the global
##			     *KeyMap after do_command(), since do_command()
##			     may change the keymap now.  Does some cursor
##			     manipulation after do_command() when at the end
##			     of the line in vi command mode, to match the
##			     behavior of vi.
##
##		* rl_getc(): Added a my declaration for $key, which was
##			     apparently omitted by the author.  rl_getc() is 
##			     no longer called directly; instead, getc_with_pending() calls
##			     it only after exhausting any requeued characters
##			     in @Pending.  @Pending is used to implement the
##			     vi '.' command, as well as the emacs DoSearch
##			     functionality.
##
##		* do_command(): Now defaults the command to 'F_Ding' if
##			     $KeyMap{default} is undefined.  This is part
##			     of the new \* feature.
##
##		* savestate()/getstate(): Now use an anonymous array instead
##		             of packing the fields into a string.
##
##		* F_AcceptLine(): Code moved to new sub add_line_to_history(),
##			     so that it may be called by F_ViSaveLine()
##			     as well as by F_AcceptLine().
##
##		* F_QuotedInsert(): Calls getc_with_pending() instead of &$rl_getc().
##
##		* F_UnixWordRubout(): Fixed bug: changed 'my' declaration of
##		             global $rl_basic_word_break_characters to 'local'.
##
##		* DoSearch(): Calls getc_with_pending() instead of &$rl_getc().  Ungets
##			     character onto @Pending instead of $pending.
##
##		* F_EmacsEditingMode(): Resets global $Vi_mode;
##
##		* F_ToggleEditingMode(): Deleted.  We use F_ViInput() and
##                           F_EmacsEditingMode() instead.
##
##		* F_PrefixMeta(): Calls getc_with_pending() instead of &$rl_getc().
##
##		* F_DigitArgument(): Calls getc_with_pending() instead of &$rl_getc().
##
##		* F_Ding(): Returns undef, for testing by vi commands.
##
##		* F_Complete(): Returns true if a completion was done, false
##                           otherwise, so vi completion routines can test it.
##
##		* complete_internal(): Returns true if a completion was done,
##                           false otherwise, so vi completion routines can
##                           test it.  Does a little cursor massaging in vi
##                           mode, to match the behavior of ksh vi mode.
##
##              Disclaimer: the original code dates from the perl 4 days, and
##              isn't very pretty by today's standards (for example,
##              extensive use of typeglobs and localized globals).  In the
##              interests of not breaking anything, I've tried to preserve
##              the old code as much as possible, and I've avoided making
##              major stylistic changes.  Since I'm not a regular emacs user,
##              I haven't done much testing to see that all the emacs-mode
##              features still work.
##
## 940817.008 - Added $var_CompleteAddsuffix.
##		Now recognizes window-change signals (at least on BSD).
##              Various typos and bug fixes.
##	Changes from Chris Arthur (csa@halcyon.com):
##		Added a few new keybindings.
##              Various typos and bug fixes.
##		Support for use from a dumb terminal.
##		Pretty-printing of filename-completion matches.
##		
## 930306.007 - Added rl_start_default_at_beginning.
##		Added optional message arg to &redisplay.
##		Added explicit numeric argument var to functions that use it.
##		Redid many commands to simplify.
##		Added TransposeChars, UpcaseWord, CapitalizeWord, DownCaseWord.
##		Redid key binding specs to better match GNU.. added
##		  undocumented "new-style" bindings.... can now bind
##		  arrow keys and other arbitrairly long key sequences.
##		Added if/else/then to .inputrc.
##		
## 930305.006 - optional "default" added (from mmuegel@cssmp.corp.mot.com).
##
## 930211.005 - fixed strange problem with eval while keybinding
##

##
## Ilya: 
##
## Added support for ReadKey, 
##
## Added customization variable $minlength
## to denote minimal lenth of a string to be put into history buffer.
##
## Added support for a bug in debugger: preinit cannot be a subroutine ?!!!
## (See immendiately below)
##
## Added support for WINCH hooks. The subroutine references should be put into
## @winchhooks.
##
## Added F_ToggleInsertMode, F_HistorySearchBackward,
## F_HistorySearchForward, PC keyboard bindings.
## 0.93: Updates to Operate, couple of keybindings added.
## $rl_completer_terminator_character, $rl_correct_sw added.
## Reload-init-file moved to C-x C-x.
## C-x ? and C-x * list/insert possible completions.

$rl_getc = \&rl_getc;

&preinit;
&init;

# # # # use strict 'vars';

# # # # # Separation into my and vars needs some thought...

# # # # use vars qw(@KeyMap %KeyMap $rl_screen_width $rl_start_default_at_beginning
# # # # 	    $rl_completion_function $rl_basic_word_break_characters
# # # # 	    $rl_completer_word_break_characters $rl_special_prefixes
# # # # 	    $rl_readline_name @rl_History $rl_MaxHistorySize
# # # #             $rl_max_numeric_arg $rl_OperateCount
# # # # 	    $KillBuffer $dumb_term $stdin_not_tty $InsertMode 
# # # # 	    $rl_NoInitFromFile);

# # # # my ($InputLocMsg, $term_OUT, $term_IN);
# # # # my ($winsz_t, $TIOCGWINSZ, $winsz, $rl_margin, $hooj, $force_redraw);
# # # # my ($hook, %var_HorizontalScrollMode, %var_EditingMode, %var_OutputMeta);
# # # # my ($var_HorizontalScrollMode, $var_EditingMode, $var_OutputMeta);
# # # # my (%var_ConvertMeta, $var_ConvertMeta, %var_MarkModifiedLines, $var_MarkModifiedLines);
# # # # my ($term_readkey, $inDOS);
# # # # my (%var_PreferVisibleBell, $var_PreferVisibleBell);
# # # # my (%var_TcshCompleteMode, $var_TcshCompleteMode);
# # # # my (%var_CompleteAddsuffix, $var_CompleteAddsuffix);
# # # # my ($minlength, @winchhooks);
# # # # my ($BRKINT, $ECHO, $FIONREAD, $ICANON, $ICRNL, $IGNBRK, $IGNCR, $INLCR,
# # # #     $ISIG, $ISTRIP, $NCCS, $OPOST, $RAW, $TCGETS, $TCOON, $TCSETS, $TCXONC,
# # # #     $TERMIOS_CFLAG, $TERMIOS_IFLAG, $TERMIOS_LFLAG, $TERMIOS_NORMAL_IOFF,
# # # #     $TERMIOS_NORMAL_ION, $TERMIOS_NORMAL_LOFF, $TERMIOS_NORMAL_LON, 
# # # #     $TERMIOS_NORMAL_OOFF, $TERMIOS_NORMAL_OON, $TERMIOS_OFLAG, 
# # # #     $TERMIOS_READLINE_IOFF, $TERMIOS_READLINE_ION, $TERMIOS_READLINE_LOFF, 
# # # #     $TERMIOS_READLINE_LON, $TERMIOS_READLINE_OOFF, $TERMIOS_READLINE_OON, 
# # # #     $TERMIOS_VMIN, $TERMIOS_VTIME, $TIOCGETP, $TIOCGWINSZ, $TIOCSETP, 
# # # #     $fion, $fionread_t, $mode, $sgttyb_t, 
# # # #     $termios, $termios_t, $winsz, $winsz_t);
# # # # my ($line, $initialized, $term_readkey);


# # # # # Global variables added for vi mode (I'm leaving them all commented
# # # # #     out, like the declarations above, until SelfLoader issues
# # # # #     are resolved).

# # # # # True when we're in one of the vi modes.
# # # # my $Vi_mode;

# # # # # Array refs: saves keystrokes for '.' command.  Undefined when we're
# # # # #     not doing a '.'-able command.
# # # # my $Dot_buf;                # Working buffer
# # # # my $Last_vi_command;        # Gets $Dot_buf when a command is parsed

# # # # # These hold state for vi 'u' and 'U'.
# # # # my($Dot_state, $Vi_undo_state, $Vi_undo_all_state);

# # # # # Refs to hashes used for cursor movement
# # # # my($Vi_delete_patterns, $Vi_move_patterns,
# # # #    $Vi_change_patterns, $Vi_yank_patterns);

# # # # # Array ref: holds parameters from the last [fFtT] command, for ';'
# # # # #     and ','.
# # # # my $Last_findchar;

# # # # # Globals for history search commands (/, ?, n, N)
# # # # my $Vi_search_re;       # Regular expression (compiled by qr{})
# # # # my $Vi_search_reverse;  # True for '?' search, false for '/'


##
## What's Cool
## ----------------------------------------------------------------------
## * hey, it's in perl.
## * Pretty full GNU readline like library...
## *	support for ~/.inputrc
## *    horizontal scrolling
## *	command/file completion
## *	rebinding
## *	history (with search)
## *	undo
## *	numeric prefixes
## * supports multi-byte characters (at least for the Japanese I use).
## * Has a tcsh-like completion-function mode.
##     call &readline'rl_set('tcsh-complete-mode', 'On') to turn on.
##

##
## What's not Cool
## ----------------------------------------------------------------------
## Can you say HUGE?
## I can't spell, so comments riddled with misspellings.
## Written by someone that has never really used readline.
## History mechanism is slightly different than GNU... may get fixed
##     someday, but I like it as it is now...
## Killbuffer not a ring.. just one level.
## Obviously not well tested yet.
## Written by someone that doesn't have a bell on his terminal, so
##     proper readline use of the bell may not be here.
##


##
## Functions beginning with F_ are functions that are mapped to keys.
## Variables and functions beginning rl_ may be accessed/set/called/read
## from outside the package.  Other things are internal.
##
## Some notable internal-only variables of global proportions:
##   $prompt -- line prompt (passed from user)
##   $line  -- the line being input
##   $D     -- ``Dot'' -- index into $line of the cursor's location.
##   $InsertMode -- usually true. False means overwrite mode.
##   $InputLocMsg -- string for error messages, such as "[~/.inputrc line 2]"
##   *emacs_keymap -- keymap for emacs-mode bindings:
##	@emacs_keymap - bindings indexed by ASCII ordinal
##      $emacs_keymap{'name'} = "emacs_keymap"
##      $emacs_keymap{'default'} = "SelfInsert"  (default binding)
##   *vi_keymap -- keymap for vi input mode bindings
##   *vicmd_keymap -- keymap for vi command mode bindings
##   *vipos_keymap -- keymap for vi positioning command bindings
##   *visearch_keymap -- keymap for vi search pattern input mode bindings
##   *KeyMap -- current keymap in effect.
##   $LastCommandKilledText -- needed so that subsequent kills accumulate
##   $lastcommand -- name of command previously run
##   $lastredisplay -- text placed upon screen during previous &redisplay
##   $si -- ``screen index''; index into $line of leftmost char &redisplay'ed
##   $force_redraw -- if set to true, causes &redisplay to be verbose.
##   $AcceptLine -- when set, its value is returned from &readline.
##   $ReturnEOF -- unless this also set, in which case undef is returned.
##   @Pending -- characters to be used as input.
##   @undo -- array holding all states of current line, for undoing.
##   $KillBuffer -- top of kill ring (well, don't have a kill ring yet)
##   @tcsh_complete_selections -- for tcsh mode, possible selections
##
## Some internal variables modified by &rl_set (see comment at &rl_set for
## info about how these set'able variables work)
##   $var_EditingMode -- a keymap typeglob like *emacs_keymap or *vi_keymap
##   $var_TcshCompleteMode -- if true, the completion function works like
##      in tcsh.  That is, the first time you try to complete something,
##	the common prefix is completed for you. Subsequent completion tries
##	(without other commands in between) cycles the command line through
##	the various possibilities.  If/when you get the one you want, just
##	continue typing.
## Other $var_ things not supported yet.
##
## Some variables used internally, but may be accessed from outside...
##   $VERSION -- just for good looks.
##   $rl_readline_name = name of program -- for .initrc if/endif stuff.
##   $rl_NoInitFromFile -- if defined when package is require'd, ~/.inputrc
##  	will not be read.
##   @rl_History -- array of previous lines input
##   $rl_HistoryIndex -- history pointer (for moving about history array)
##   $rl_completion_function -- see "How Command Completion Works" (way) below.
##   $rl_basic_word_break_characters -- string of characters that can cause
##	a word break for forward-word, etc.
##   $rl_start_default_at_beginning --
##	Normally, the user's cursor starts at the end of any default text
##	passed to readline.  If this variable is true, it starts at the
##	beginning.
##   $rl_completer_word_break_characters --
##	like $rl_basic_word_break_characters (and in fact defaults to it),
##	but for the completion function.
##   $rl_completer_terminator_character -- what to insert to separate
##      a completed token from the rest.  Reset at beginning of
##      completion to ' ' so completion function can change it.
##   $rl_special_prefixes -- characters that are part of this string as well
##      as of $rl_completer_word_break_characters cause a word break for the
##	completer function, but remain part of the word.  An example: consider
##      when the input might be perl code, and one wants to be able to
##      complete on variable and function names, yet still have the '$',
##	'&', '@',etc. part of the $text to be completed. Then set this var
## 	to '&@$%' and make sure each of these characters is in
## 	$rl_completer_word_break_characters as well....
##   $rl_MaxHistorySize -- maximum size that the history array may grow.
##   $rl_screen_width -- width readline thinks it can use on the screen.
##   $rl_correct_sw -- is substructed from the real width of the terminal
##   $rl_margin -- when moving to within this far from a margin, scrolls.
##   $rl_CLEAR -- what to output to clear the screen.
##   $rl_max_numeric_arg -- maximum numeric arg allowed.
##

sub get_window_size
{
    my $sig = shift;
    my ($num_cols,$num_rows);
    
    if (defined $term_readkey) {
	 ($num_cols,$num_rows) =  Term::ReadKey::GetTerminalSize($term_OUT);
	 $rl_screen_width = $num_cols - $rl_correct_sw
	   if defined($num_cols) && $num_cols;
    } elsif (ioctl($term_IN,$TIOCGWINSZ,$winsz)) {
	 ($num_rows,$num_cols) = unpack($winsz_t,$winsz);
	 $rl_screen_width = $num_cols - $rl_correct_sw
	   if defined($num_cols) && $num_cols;
    }
    $rl_margin = int($rl_screen_width/3);
    if (defined $sig) {
	$force_redraw = 1;
	&redisplay();
    }
    
    for $hook (@winchhooks) {
      eval {&$hook()}; warn $@ if $@ and $^W;
    }
    local $^W = 0;		# WINCH may be illegal...
    $SIG{'WINCH'} = "readline::get_window_size";
}

sub preinit
{
    ## Set up the input and output handles

    $term_IN = \*STDIN unless defined $term_IN;
    $term_OUT = \*STDOUT unless defined $term_OUT;
    ## not yet supported... always on.
    $var_HorizontalScrollMode = 1;
    $var_HorizontalScrollMode{'On'} = 1;
    $var_HorizontalScrollMode{'Off'} = 0;

    $var_EditingMode{'emacs'}    = *emacs_keymap;
    $var_EditingMode{'vi'}       = *vi_keymap;
    $var_EditingMode{'vicmd'}    = *vicmd_keymap;
    $var_EditingMode{'vipos'}    = *vipos_keymap;
    $var_EditingMode{'visearch'} = *visearch_keymap;

    ## not yet supported... always on
    $var_InputMeta = 1;
    $var_InputMeta{'Off'} = 0;
    $var_InputMeta{'On'} = 1;

    ## not yet supported... always on
    $var_OutputMeta = 1;
    $var_OutputMeta{'Off'} = 0;
    $var_OutputMeta{'On'} = 1;

    ## not yet supported... always off
    $var_ConvertMeta = 0;
    $var_ConvertMeta{'Off'} = 0;
    $var_ConvertMeta{'On'} = 1;

    ## not yet supported... always off
    $var_MetaFlag = 0;
    $var_MetaFlag{'Off'} = 0;
    $var_MetaFlag{'On'} = 1;

    ## not yet supported... always off
    $var_MarkModifiedLines = 0;
    $var_MarkModifiedLines{'Off'} = 0;
    $var_MarkModifiedLines{'On'} = 1;

    ## not yet supported... always off
    $var_PreferVisibleBell = 0;
    $var_PreferVisibleBell{'On'} = 1;
    $var_PreferVisibleBell{'Off'} = 0;

    ## this is an addition. Very nice.
    $var_TcshCompleteMode = 0;
    $var_TcshCompleteMode{'On'} = 1;
    $var_TcshCompleteMode{'Off'} = 0;

    $var_CompleteAddsuffix = 1;
    $var_CompleteAddsuffix{'On'} = 1;
    $var_CompleteAddsuffix{'Off'} = 0;

    # To conform to interface
    $minlength = 1 unless defined $minlength;

    # WINCH hooks
    @winchhooks = ();

    $inDOS = $^O eq 'os2' || defined $ENV{OS2_SHELL} unless defined $inDOS;
    eval {
      require Term::ReadKey; $term_readkey++;
    };
    if ($@) {
      eval {require "ioctl.pl"}; ## try to get, don't die if not found.
      eval {require "sys/ioctl.ph"}; ## try to get, don't die if not found.
      eval {require "sgtty.ph"}; ## try to get, don't die if not found.
      if ($inDOS and !defined $TIOCGWINSZ) {
	  $TIOCGWINSZ=0;
	  $TIOCGETP=1;
	  $TIOCSETP=2;
	  $sgttyb_t="I5 C8";
	  $winsz_t="";
	  $RAW=0xf002;
	  $ECHO=0x0008;
      }
      $TIOCGETP = &TIOCGETP if defined(&TIOCGETP);
      $TIOCSETP = &TIOCSETP if defined(&TIOCSETP);
      $TIOCGWINSZ = &TIOCGWINSZ if defined(&TIOCGWINSZ);
      $FIONREAD = &FIONREAD if defined(&FIONREAD);
      $TCGETS = &TCGETS if defined(&TCGETS);
      $TCSETS = &TCSETS if defined(&TCSETS);
      $TCXONC = &TCXONC if defined(&TCXONC);
      $TIOCGETP   = 0x40067408 if !defined($TIOCGETP);
      $TIOCSETP   = 0x80067409 if !defined($TIOCSETP);
      $TIOCGWINSZ = 0x40087468 if !defined($TIOCGWINSZ);
      $FIONREAD   = 0x4004667f if !defined($FIONREAD);
      $TCGETS     = 0x40245408 if !defined($TCGETS);
      $TCSETS     = 0x80245409 if !defined($TCSETS);
      $TCXONC     = 0x20005406 if !defined($TCXONC);

      ## TTY modes
      $ECHO = &ECHO if defined(&ECHO);
      $RAW = &RAW if defined(&RAW);
      $RAW	= 040 if !defined($RAW);
      $ECHO	= 010 if !defined($ECHO);
      #$CBREAK    = 002 if !defined($CBREAK);
      $mode = $RAW; ## could choose CBREAK for testing....

      $IGNBRK     = 1 if !defined($IGNBRK);
      $BRKINT     = 2 if !defined($BRKINT);
      $ISTRIP     = 040 if !defined($ISTRIP);
      $INLCR      = 0100 if !defined($INLCR);
      $IGNCR      = 0200 if !defined($IGNCR);
      $ICRNL      = 0400 if !defined($ICRNL);
      $OPOST      = 1 if !defined($OPOST);
      $ISIG       = 1 if !defined($ISIG);
      $ICANON     = 2 if !defined($ICANON);
      $TCOON      = 1 if !defined($TCOON);
      $TERMIOS_READLINE_ION = $BRKINT;
      $TERMIOS_READLINE_IOFF = $IGNBRK | $ISTRIP | $INLCR | $IGNCR | $ICRNL;
      $TERMIOS_READLINE_OON = 0;
      $TERMIOS_READLINE_OOFF = $OPOST;
      $TERMIOS_READLINE_LON = 0;
      $TERMIOS_READLINE_LOFF = $ISIG | $ICANON | $ECHO;
      $TERMIOS_NORMAL_ION = $BRKINT;
      $TERMIOS_NORMAL_IOFF = $IGNBRK;
      $TERMIOS_NORMAL_OON = $OPOST;
      $TERMIOS_NORMAL_OOFF = 0;
      $TERMIOS_NORMAL_LON = $ISIG | $ICANON | $ECHO;
      $TERMIOS_NORMAL_LOFF = 0;

      #$sgttyb_t   = 'C4 S';
      #$winsz_t = "S S S S";  # rows,cols, xpixel, ypixel
      $sgttyb_t   = 'C4 S' if !defined($sgttyb_t);
      $winsz_t = "S S S S" if !defined($winsz_t);  
      # rows,cols, xpixel, ypixel
      $winsz = pack($winsz_t,0,0,0,0);
      $fionread_t = "L";
      $fion = pack($fionread_t, 0);
      $NCCS = 17;
      $termios_t = "LLLLc" . ("c" x $NCCS);  # true for SunOS 4.1.3, at least...
      $termios = ''; ## just to shut up "perl -w".
      $termios = pack($termios, 0);  # who cares, just make it long enough
      $TERMIOS_IFLAG = 0;
      $TERMIOS_OFLAG = 1;
      $TERMIOS_CFLAG = 2;
      $TERMIOS_LFLAG = 3;
      $TERMIOS_VMIN = 5 + 4;
      $TERMIOS_VTIME = 5 + 5;
    }
    $rl_correct_sw = ($inDOS ? 1 : 0);

    $rl_start_default_at_beginning = 0;
    $rl_screen_width = 79; ## default

    $rl_completion_function = "rl_filename_list"
	unless defined($rl_completion_function);
    $rl_basic_word_break_characters = "\\\t\n' \"`\@\$><=;|&{(";
    $rl_completer_word_break_characters = $rl_basic_word_break_characters;
    $rl_special_prefixes = '';
    ($rl_readline_name = $0) =~ s#.*[/\\]## if !defined($rl_readline_name);

    @rl_History=() if !(@rl_History);
    $rl_MaxHistorySize = 100 if !defined($rl_MaxHistorySize);
    $rl_max_numeric_arg = 200 if !defined($rl_max_numeric_arg);
    $rl_OperateCount = 0 if !defined($rl_OperateCount);

    $rl_term_set = \@Term::ReadLine::TermCap::rl_term_set;
    @$rl_term_set or $rl_term_set = ["","","",""];

    $InsertMode=1;
    $KillBuffer='';
    $line='';
    $D = 0;
    $InputLocMsg = ' [initialization]';
    
    &InitKeymap(*emacs_keymap, 'SelfInsert', 'emacs_keymap',
		($inDOS ? () : ('C-@',	'Ding') ),
		'C-a',	'BeginningOfLine',
		'C-b',	'BackwardChar',
		'C-c',	'Interrupt',
		'C-d',	'DeleteChar',
		'C-e',	'EndOfLine',
		'C-f',	'ForwardChar',
		'C-g',	'Abort',
		'M-C-g',	'Abort',
		'C-h',	'BackwardDeleteChar',
		"TAB" ,	'Complete',
		"C-j" ,	'AcceptLine',
		'C-k',	'KillLine',
		'C-l',	'ClearScreen',
		"C-m" ,	'AcceptLine',
		'C-n',	'NextHistory',
		'C-o',  'OperateAndGetNext',
		'C-p',	'PreviousHistory',
		'C-q',	'QuotedInsert',
		'C-r',	'ReverseSearchHistory',
		'C-s',	'ForwardSearchHistory',
		'C-t',	'TransposeChars',
		'C-u',	'UnixLineDiscard',
		##'C-v',	'QuotedInsert',
		'C-v',	'HistorySearchForward',
		'C-w',	'UnixWordRubout',
		qq/"\cX\cX"/,	'ReReadInitFile',
		qq/"\cX?"/,	'PossibleCompletions',
		qq/"\cX*"/,	'InsertPossibleCompletions',
		'C-y',	'Yank',
		'C-z',	'Suspend',
		'C-\\',	'Ding',
		'C-^',	'Ding',
		'C-_',	'Undo',
		'DEL',	($inDOS ?
			 'BackwardKillWord' : # <Control>+<Backspace>
			 'BackwardDeleteChar'
			),
		'M-<',	'BeginningOfHistory',
		'M->',	'EndOfHistory',
		'M-DEL',	'BackwardKillWord',
		'M-C-h',	'BackwardKillWord',
		'M-C-j',	'ViInput',
		'M-C-v',	'QuotedInsert',
		'M-b',	'BackwardWord',
		'M-c',	'CapitalizeWord',
		'M-d',	'KillWord',
		'M-f',	'ForwardWord',
		'M-l',	'DownCaseWord',
		'M-r',	'RevertLine',
		'M-t',	'TransposeWords',
		'M-u',	'UpcaseWord',
		'M-v',	'HistorySearchBackward',
		'M-y',	'YankPop',
		"M-?",	'PossibleCompletions',
		"M-TAB",	'TabInsert',
		qq/"\e[A"/,  'previous-history',
		qq/"\e[B"/,  'next-history',
		qq/"\e[C"/,  'forward-char',
		qq/"\e[D"/,  'backward-char',
		qq/"\eOA"/,  'previous-history',
		qq/"\eOB"/,  'next-history',
		qq/"\eOC"/,  'forward-char',
		qq/"\eOD"/,  'backward-char',
		qq/"\e[[A"/,  'previous-history',
		qq/"\e[[B"/,  'next-history',
		qq/"\e[[C"/,  'forward-char',
		qq/"\e[[D"/,  'backward-char',
		qq/"\e[2~"/,   'ToggleInsertMode', # X: <Insert>

		# HP xterm
		#qq/"\e[A"/,   'PreviousHistory',	# up    arrow
		#qq/"\e[B"/,   'NextHistory',		# down  arrow
		#qq/"\e[C"/,   'ForwardChar',		# right arrow
		#qq/"\e[D"/,   'BackwardChar',		# left  arrow
		qq/"\e[H"/,   'BeginningOfLine',        # home
		qq/"\e[1~"/,  'HistorySearchForward',   # find
		qq/"\e[3~"/,  'ToggleInsertMode',	# insert char
		qq/"\e[4~"/,  'ToggleInsertMode',	# select
		qq/"\e[5~"/,  'HistorySearchBackward',	# prev
		qq/"\e[6~"/,  'HistorySearchForward',	# next
		qq/"\e[\0"/,  'BeginningOfLine',	# home
		#'C-k',        'KillLine',		# clear display

		# hpterm

		($ENV{'TERM'} eq 'hpterm' ?
		 (
		  qq/"\eA"/,    'PreviousHistory',     # up    arrow
		  qq/"\eB"/,    'NextHistory',	       # down  arrow
		  qq/"\eC"/,    'ForwardChar',	       # right arrow
		  qq/"\eD"/,    'BackwardChar',	       # left  arrow
		  qq/"\eS"/,    'BeginningOfHistory',  # shift up    arrow
		  qq/"\eT"/,    'EndOfHistory',	       # shift down  arrow
		  qq/"\e&r1R"/, 'EndOfLine',	       # shift right arrow
		  qq/"\e&r1L"/, 'BeginningOfLine',     # shift left  arrow
		  qq/"\eJ"/,    'ClearScreen',	       # clear display
		  qq/"\eM"/,    'UnixLineDiscard',     # delete line
		  qq/"\eK"/,    'KillLine',	       # clear  line
		  qq/"\eG\eK"/, 'BackwardKillLine',    # shift clear line
		  qq/"\eP"/,    'DeleteChar',	       # delete char
		  qq/"\eL"/,    'Yank',		       # insert line
		  qq/"\eQ"/,    'ToggleInsertMode',    # insert char
		  qq/"\eV"/,    'HistorySearchBackward',# prev
		  qq/"\eU"/,    'HistorySearchForward',# next
		  qq/"\eh"/,    'BeginningOfLine',     # home
		  qq/"\eF"/,    'EndOfLine',	       # shift home
		  qq/"\ei"/,    'Suspend',	       # shift tab
		 ) :
		 ()
		),
		($inDOS ?
		 (
		  qq/"\0\16"/, 'Undo', # 14: <Alt>+<Backspace>
		  qq/"\0\23"/, 'RevertLine', # 19: <Alt>+<R>
		  qq/"\0\24"/, 'TransposeWords', # 20: <Alt>+<T>
		  qq/"\0\25"/, 'YankPop', # 21: <Alt>+<Y>
		  qq/"\0\26"/, 'UpcaseWord', # 22: <Alt>+<U>
		  qq/"\0\31"/, 'ReverseSearchHistory', # 25: <Alt>+<P>
		  qq/"\0\40"/, 'KillWord', # 32: <Alt>+<D>
		  qq/"\0\41"/, 'ForwardWord', # 33: <Alt>+<F>
		  qq/"\0\46"/, 'DownCaseWord', # 38: <Alt>+<L>
		  #qq/"\0\51"/, 'TildeExpand', # 41: <Alt>+<\'>
		  qq/"\0\56"/, 'CapitalizeWord', # 46: <Alt>+<C>
		  qq/"\0\60"/, 'BackwardWord', # 48: <Alt>+<B>
		  qq/"\0\61"/, 'ForwardSearchHistory', # 49: <Alt>+<N>
		  #qq/"\0\64"/, 'YankLastArg', # 52: <Alt>+<.>
		  qq/"\0\65"/, 'PossibleCompletions', # 53: <Alt>+</>
		  qq/"\0\107"/, 'BeginningOfLine', # 71: <Home>
		  qq/"\0\110"/, 'previous-history', # 72: <Up arrow>
		  qq/"\0\111"/, 'HistorySearchBackward', # 73: <Page Up>
		  qq/"\0\113"/, 'backward-char', # 75: <Left arrow>
		  qq/"\0\115"/, 'forward-char', # 77: <Right arrow>
		  qq/"\0\117"/, 'EndOfLine', # 79: <End>
		  qq/"\0\120"/, 'next-history', # 80: <Down arrow>
		  qq/"\0\121"/, 'HistorySearchForward', # 81: <Page Down>
		  qq/"\0\122"/, 'ToggleInsertMode', # 82: <Insert>
		  qq/"\0\123"/, 'DeleteChar', # 83: <Delete>
		  qq/"\0\163"/, 'BackwardWord', # 115: <Ctrl>+<Left arrow>
		  qq/"\0\164"/, 'ForwardWord', # 116: <Ctrl>+<Right arrow>
		  qq/"\0\165"/, 'KillLine', # 117: <Ctrl>+<End>
		  qq/"\0\166"/, 'EndOfHistory', # 118: <Ctrl>+<Page Down>
		  qq/"\0\167"/, 'BackwardKillLine', # 119: <Ctrl>+<Home>
		  qq/"\0\204"/, 'BeginningOfHistory', # 132: <Ctrl>+<Page Up>
		  qq/"\0\223"/, 'KillWord', # 147: <Ctrl>+<Delete>
		 )
		 : ( 'C-@',	'Ding')
		)
	       );

    *KeyMap = *emacs_keymap;
    my @add_bindings = ();
    foreach ('-', '0' .. '9') { push(@add_bindings, "M-$_", 'DigitArgument'); }
    foreach ("A" .. "Z") {
      next if  # defined($KeyMap[27]) && defined (%{"$KeyMap{name}_27"}) &&
	defined $ {"$KeyMap{name}_27"}[ord $_];
      push(@add_bindings, "M-$_", 'DoLowercaseVersion');
    }
    &rl_bind(@add_bindings);
    
    # Vi input mode.
    &InitKeymap(*vi_keymap, 'SelfInsert', 'vi_keymap',

		"\e",	'ViEndInsert',
		'C-c',	'Interrupt',
		'C-h',	'BackwardDeleteChar',
		'C-w',	'UnixWordRubout',
		'C-u',	'UnixLineDiscard',
		'C-v',	'QuotedInsert',
		'DEL',	'BackwardDeleteChar',
		"\n",	'ViAcceptInsert',
		"\r",	'ViAcceptInsert',
	       );

    # Vi command mode.
    &InitKeymap(*vicmd_keymap, 'Ding', 'vicmd_keymap',

		'C-c',	'Interrupt',
		'C-e',	'EmacsEditingMode',
		'M-C-j','EmacsEditingMode',
		'C-h',	'ViMoveCursor',
		'C-l',	'ClearScreen',
		"\n",	'ViAcceptLine',
		"\r",	'ViAcceptLine',

		' ',	'ViMoveCursor',
		'#',	'ViSaveLine',
		'$',	'ViMoveCursor',
		'%',	'ViMoveCursor',
		'*',    'ViInsertPossibleCompletions',
		'+',	'ViNextHistory',
		',',	'ViMoveCursor',
		'-',	'ViPreviousHistory',
		'.',	'ViRepeatLastCommand',
		'/',	'ViSearch',

		'0',	'ViMoveCursor',
		'1',	'ViDigit',
		'2',	'ViDigit',
		'3',	'ViDigit',
		'4',	'ViDigit',
		'5',	'ViDigit',
		'6',	'ViDigit',
		'7',	'ViDigit',
		'8',	'ViDigit',
		'9',	'ViDigit',

		';',	'ViMoveCursor',
		'=',    'ViPossibleCompletions',
		'?',	'ViSearch',

		'A',	'ViAppendLine',
		'B',	'ViMoveCursor',
		'C',	'ViChangeLine',
		'D',	'ViDeleteLine',
		'E',	'ViMoveCursor',
		'F',	'ViMoveCursor',
		'G',	'ViHistoryLine',
		'H',	'ViPrintHistory',
		'I',	'ViBeginInput',
		'N',	'ViRepeatSearch',
		'P',	'ViPutBefore',
		'R',	'ViReplaceMode',
		'S',	'ViChangeEntireLine',
		'T',	'ViMoveCursor',
		'U',	'ViUndoAll',
		'W',	'ViMoveCursor',
		'X',	'ViBackwardDeleteChar',
		'Y',	'ViYankLine',

		'\\',   'ViComplete',
		'^',	'ViMoveCursor',

		'a',	'ViAppend',
		'b',	'ViMoveCursor',
		'c',	'ViChange',
		'd',	'ViDelete',
		'e',	'ViMoveCursor',
		'f',	'ViMoveCursorFind',
		'h',	'ViMoveCursor',
		'i',	'ViInput',
		'j',	'ViNextHistory',
		'k',	'ViPreviousHistory',
		'l',	'ViMoveCursor',
		'n',	'ViRepeatSearch',
		'p',	'ViPut',
		'r',	'ViReplaceChar',
		's',	'ViChangeChar',
		't',	'ViMoveCursorTo',
		'u',	'ViUndo',
		'w',	'ViMoveCursor',
		'x',	'ViDeleteChar',
		'y',	'ViYank',

		'|',	'ViMoveCursor',
		'~',	'ViToggleCase',

		($inDOS ?
		 (
		  qq/"\0\110"/, 'ViPreviousHistory',   # 72: <Up arrow>
		  qq/"\0\120"/, 'ViNextHistory',       # 80: <Down arrow>
		  qq/"\0\113"/, 'BackwardChar',        # 75: <Left arrow>
		  qq/"\0\115"/, 'ForwardChar',         # 77: <Right arrow>
		  "\e",	        'ViCommandMode',
		 ) :

		($ENV{'TERM'} eq 'hpterm') ?
		 (
		  qq/"\eA"/,    'ViPreviousHistory',   # up    arrow
		  qq/"\eB"/,    'ViNextHistory',       # down  arrow
		  qq/"\eC"/,    'ForwardChar',	       # right arrow
		  qq/"\eD"/,    'BackwardChar',	       # left  arrow
		  qq/"\e\\*"/,  'ViAfterEsc',
		 ) :

		# Default
		 (
		  qq/"\e[A"/,   'ViPreviousHistory',	# up    arrow
		  qq/"\e[B"/,   'ViNextHistory',	# down  arrow
		  qq/"\e[C"/,   'ForwardChar',		# right arrow
		  qq/"\e[D"/,   'BackwardChar',		# left  arrow
		  qq/"\e\\*"/,  'ViAfterEsc', 
		  qq/"\e[\\*"/, 'ViAfterEsc', 
		 )
		),
	       );

    # Vi positioning commands (suffixed to vi commands like 'd').
    &InitKeymap(*vipos_keymap, 'ViNonPosition', 'vipos_keymap',

		'^',	'ViFirstWord',
		'0',	'BeginningOfLine',
		'1',	'ViDigit',
		'2',	'ViDigit',
		'3',	'ViDigit',
		'4',	'ViDigit',
		'5',	'ViDigit',
		'6',	'ViDigit',
		'7',	'ViDigit',
		'8',	'ViDigit',
		'9',	'ViDigit',
		'$',	'EndOfLine',
		'h',	'BackwardChar',
		'l',	'ForwardChar',
		' ',	'ForwardChar',
		'C-h',	'BackwardChar',
		'f',	'ViForwardFindChar',
		'F',	'ViBackwardFindChar',
		't',	'ViForwardToChar',
		'T',	'ViBackwardToChar',
		';',	'ViRepeatFindChar',
		',',	'ViInverseRepeatFindChar',
		'%',	'ViFindMatchingParens',
		'|',	'ViMoveToColumn',

		# Arrow keys
		($inDOS ?
		 (
		  qq/"\0\115"/, 'ForwardChar',         # 77: <Right arrow>
		  qq/"\0\113"/, 'BackwardChar',        # 75: <Left arrow>
		  "\e",	        'ViPositionEsc',
		 ) :

		($ENV{'TERM'} eq 'hpterm') ?
		 (
		  qq/"\eC"/,    'ForwardChar',	       # right arrow
		  qq/"\eD"/,    'BackwardChar',	       # left  arrow
		  qq/"\e\\*"/,  'ViPositionEsc',
		 ) :

		# Default
		 (
		  qq/"\e[C"/,   'ForwardChar',		# right arrow
		  qq/"\e[D"/,   'BackwardChar',		# left  arrow
		  qq/"\e\\*"/,  'ViPositionEsc',
		  qq/"\e[\\*"/, 'ViPositionEsc',
		 )
		),
	       );

    # Vi search string input mode for '/' and '?'.
    &InitKeymap(*visearch_keymap, 'SelfInsert', 'visearch_keymap',

		"\e",	'Ding',
		'C-c',	'Interrupt',
		'C-h',	'ViSearchBackwardDeleteChar',
		'C-w',	'UnixWordRubout',
		'C-u',	'UnixLineDiscard',
		'C-v',	'QuotedInsert',
		'DEL',	'ViSearchBackwardDeleteChar',
		"\n",	'ViEndSearch',
		"\r",	'ViEndSearch',
	       );

    # These constant hashes hold the arguments to &forward_scan() or
    #     &backward_scan() for vi positioning commands, which all
    #     behave a little differently for delete, move, change, and yank.
    #
    # Note: I originally coded these as qr{}, but changed them to q{} for
    #       compatibility with older perls at the expense of some performance.
    #
    # Note: Some of the more obscure key combinations behave slightly
    #       differently in different vi implementation.  This module matches
    #       the behavior of /usr/ucb/vi, which is different from the
    #       behavior of vim, nvi, and the ksh command line.  One example is
    #       the command '2de', when applied to the string ('^' represents the
    #       cursor, not a character of the string):
    #
    #           ^5.6   7...88888888
    #
    #       With /usr/ucb/vi and with this module, the result is
    #
    #           ^...88888888
    #
    #       but with the other three vi implementations, the result is
    #
    #           ^   7...88888888

    $Vi_delete_patterns = {
	ord('w')  =>  q{(?:\w+|[^\w\s]+|)\s*},
	ord('W')  =>  q{\S*\s*},
	ord('b')  =>  q{\w+\s*|[^\w\s]+\s*|^\s+},
	ord('B')  =>  q{\S+\s*|^\s+},
	ord('e')  =>  q{.\s*\w+|.\s*[^\w\s]+|.\s*$},
	ord('E')  =>  q{.\s*\S+|.\s*$},
    };

    $Vi_move_patterns = {
	ord('w')  =>  q{(?:\w+|[^\w\s]+|)\s*},
	ord('W')  =>  q{\S*\s*},
	ord('b')  =>  q{\w+\s*|[^\w\s]+\s*|^\s+},
	ord('B')  =>  q{\S+\s*|^\s+},
	ord('e')  =>  q{.\s*\w*(?=\w)|.\s*[^\w\s]*(?=[^\w\s])|.?\s*(?=\s$)},
	ord('E')  =>  q{.\s*\S*(?=\S)|.?\s*(?=\s$)},
    };

    $Vi_change_patterns = {
	ord('w')  =>  q{\w+|[^\w\s]+|\s},
	ord('W')  =>  q{\S+|\s},
	ord('b')  =>  q{\w+\s*|[^\w\s]+\s*|^\s+},
	ord('B')  =>  q{\S+\s*|^\s+},
	ord('e')  =>  q{.\s*\w+|.\s*[^\w\s]+|.\s*$},
	ord('E')  =>  q{.\s*\S+|.\s*$},
    };

    $Vi_yank_patterns = {
	ord('w')  =>  q{(?:\w+|[^\w\s]+|)\s*},
	ord('W')  =>  q{\S*\s*},
	ord('b')  =>  q{\w+\s*|[^\w\s]+\s*|^\s+},
	ord('B')  =>  q{\S+\s*|^\s+},
	ord('e')  =>  q{.\s*\w*(?=\w)|.\s*[^\w\s]*(?=[^\w\s])|.?\s*(?=\s$)},
	ord('E')  =>  q{.\s*\S*(?=\S)|.?\s*(?=\s$)},
    };

    my $default_mode =
	(defined $ENV{EDITOR} and $ENV{EDITOR} =~ /vi/) ? 'vi' : 'emacs';

    *KeyMap = $var_EditingMode = $var_EditingMode{$default_mode};

    1;				# Returning a glob causes a bug in db5.001m
}

sub init
{
    if ($ENV{'TERM'} eq 'emacs' || $ENV{'TERM'} eq 'dumb') {
	$dumb_term = 1;
    } elsif (! -c $term_IN && $term_IN eq \*STDIN) { # Believe if it is given
    	$stdin_not_tty = 1;
    } else {
	&get_window_size;
	&F_ReReadInitFile if !defined($rl_NoInitFromFile);
	$InputLocMsg = '';
	*KeyMap = $var_EditingMode;
    }

    $initialized = 1;
}


##
## InitKeymap(*keymap, 'default', 'name', bindings.....)
##
sub InitKeymap
{
    local(*KeyMap) = shift(@_);
    my $default = shift(@_);
    my $name = $KeyMap{'name'} = shift(@_);

    # 'default' is now optional - if '', &do_command() defaults it to
    #     'F_Ding'.  Meta-maps now don't set a default - this lets
    #     us detect multiple '\*' default declarations.              JP
    if ($default ne '') {
	my $func = $KeyMap{'default'} = "F_$default";
	### Temporarily disabled
	die qq/Bad default function [$func] for keymap "$name"/
	  if !$autoload_broken and !defined(&$func);
    }

    &rl_bind if @_ > 0;	## The rest of @_ gets passed silently.
}

##
## Accepts an array as pairs ($keyspec, $function, [$keyspec, $function]...).
## and maps the associated bindings to the current KeyMap.
##
## keyspec should be the name of key sequence in one of two forms:
##
## Old (GNU readline documented) form:
##	     M-x	to indicate Meta-x
##	     C-x	to indicate Ctrl-x
##	     M-C-x	to indicate Meta-Ctrl-x
##	     x		simple char x
##      where 'x' above can be a single character, or the special:
##          special  	means
##         --------  	-----
##	     space	space   ( )
##	     spc	space   ( )
##	     tab	tab     (\t)
##	     del	delete  (0x7f)
##	     rubout	delete  (0x7f)
##	     newline 	newline (\n)
##	     lfd     	newline (\n)
##	     ret     	return  (\r)
##	     return  	return  (\r)
##	     escape  	escape  (\e)
##	     esc     	escape  (\e)
##
## New form:
##	  "chars"   (note the required double-quotes)
##   where each char in the list represents a character in the sequence, except
##   for the special sequences:
##	  \\C-x		Ctrl-x
##	  \\M-x		Meta-x
##	  \\M-C-x	Meta-Ctrl-x
##	  \\e		escape.
##	  \\x		x (if not one of the above)
##
##
## FUNCTION should be in the form 'BeginningOfLine' or 'beginning-of-line'.
## It is an error for the function to not be known....
##
## As an example, the following lines in .inputrc will bind one's xterm
## arrow keys:
##     "\e[[A": previous-history
##     "\e[[B": next-history
##     "\e[[C": forward-char
##     "\e[[D": backward-char
##

sub actually_do_binding
{
  ##
  ## actually_do_binding($function1, \@sequence1, ...)
  ##
  ## Actually inserts the binding for @sequence to $function into the
  ## current map.  @sequence is an array of character ordinals.
  ##
  ## If @sequence is more than one element long, all but the last will
  ## cause meta maps to be created.
  ##
  ## $Function will have an implicit "F_" prepended to it.
  ##
  while (@_) {
    my $func = shift;
    my ($key, @keys) = @{shift()};
    $key += 0;
    local(*KeyMap) = *KeyMap;
    my $map;
    while (@keys) {
      if (defined($KeyMap[$key]) && ($KeyMap[$key] ne 'F_PrefixMeta')) {
	warn "Warning$InputLocMsg: ".
	  "Re-binding char #$key from [$KeyMap[$key]] to meta.\n" if $^W;
      }
      $KeyMap[$key] = 'F_PrefixMeta';
      $map = "$KeyMap{'name'}_$key";
      InitKeymap(*$map, '', $map) if !(%$map);
      *KeyMap = *$map;
      $key = shift @keys;
      #&actually_do_binding($func, \@keys);
    }

    my $name = $KeyMap{'name'};
    if ($key eq 'default') {      # JP: added
	warn "Warning$InputLocMsg: ".
	  " changing default action to $func in $name key map\n"
	  if $^W && defined $KeyMap{'default'};

	$KeyMap{'default'} = "F_$func";
    }
    else {
	if (defined($KeyMap[$key]) && $KeyMap[$key] eq 'F_PrefixMeta'
	    && $func ne 'PrefixMeta')
	  {
	    warn "Warning$InputLocMsg: ".
	      " Re-binding char #$key to non-meta ($func) in $name key map\n"
	      if $^W;
	  }
	$KeyMap[$key] = "F_$func";
    }
  }
}

sub rl_bind
{
    my (@keys, $key, $func, $ord, @arr);

    while (defined($key = shift(@_)) && defined($func = shift(@_)))
    {
	##
	## Change the function name from something like
	##	backward-kill-line
	## to
	##	BackwardKillLine
	## if not already there.
	##
	$func = "\u$func";
	$func =~ s/-(.)/\u$1/g;		

	# Temporary disabled
	if (!$autoload_broken and !defined($ {'readline::'}{"F_$func"})) {
	    warn "Warning$InputLocMsg: bad bind function [$func]\n" if $^W;
	    next;
	}

	## print "sequence [$key] func [$func]\n"; ##DEBUG

	@keys = ();
 	## See if it's a new-style binding.
	if ($key =~ m/"(.*[^\\])"/) {
	    $key = $1;
	    ## New-style bindings are enclosed in double-quotes.
	    ## Characters are taken verbatim except the special cases:
	    ##    \C-x    Control x (for any x)
	    ##    \M-x    Meta x (for any x)
	    ##    \e	  Escape
	    ##    \*      Set the keymap default   (JP: added this)
	    ##               (must be the last character of the sequence)
	    ##
	    ##    \x      x  (unless it fits the above pattern)
	    ##
	    ## Look for special case of "\C-\M-x", which should be treated
	    ## like "\M-\C-x".
	    
	    while (length($key) > 0) {

		# JP: fixed regex bugs below: changed all 's#' to 's#^'

		if ($key =~ s#^\\C-\\M-(.)##) {
		   push(@keys, ord("\e"), &ctrl(ord($1)));
		} elsif ($key =~ s#^\\(M-|e)##) {
		   push(@keys, ord("\e"));
		} elsif ($key =~ s#^\\C-(.)##) {
		   push(@keys, &ctrl(ord($1)));
		} elsif ($key =~ s#^\\x([0-9a-fA-F]{2})##) {
		   push(@keys, eval('0x'.$1));
		} elsif ($key =~ s#^\\\*$##) {    # JP: added
		   push(@keys, 'default');
		} elsif ($key =~ s#^\\(.)##) {
		   push(@keys, ord($1));
		} else {
		   push(@keys, ord($key));
		   substr($key,0,1) = '';
		}
	    }
	} else {
	    ## ol-dstyle binding... only one key (or Meta+key)
	    my ($isctrl, $orig) = (0, $key);
	    $isctrl = $key =~ s/(C|Control|CTRL)-//i;
	    push(@keys, ord("\e")) if $key =~ s/(M|Meta)-//i; ## is meta?
	    ## Isolate key part. This matches GNU's implementation.
	    ## If the key is '-', be careful not to delete it!
	    $key =~ s/.*-(.)/$1/;
	    if    ($key =~ /^(space|spc)$/i)   { $key = ' ';    }
	    elsif ($key =~ /^(rubout|del)$/i)  { $key = "\x7f"; }
	    elsif ($key =~ /^tab$/i)           { $key = "\t";   }
	    elsif ($key =~ /^(return|ret)$/i)  { $key = "\r";   }
	    elsif ($key =~ /^(newline|lfd)$/i) { $key = "\n";   }
	    elsif ($key =~ /^(escape|esc)$/i)  { $key = "\e";   }
	    elsif (length($key) > 1) {
	        warn "Warning$InputLocMsg: strange binding [$orig]\n" if $^W;
	    }
	    $key = ord($key);
	    $key = &ctrl($key) if $isctrl;
	    push(@keys, $key);
	}

	# 
	## Now do the mapping of the sequence represented in @keys
	 #
	# print "&actually_do_binding($func, @keys)\n"; ##DEBUG
	push @arr, $func, [@keys];
	#&actually_do_binding($func, \@keys);
    }
    &actually_do_binding(@arr);
}

sub F_ReReadInitFile
{
    my ($file) = $ENV{'INPUTRC'};
    $file = $ENV{'HOME'}."/.inputrc" unless defined $file;
    return if !open(RC, $file);
    my (@action) = ('exec'); ## exec, skip, ignore (until appropriate endif)
    my (@level) = ();        ## if, else

    while (<RC>) {
	s/^\s*//;
	next if m/^\s*(#|$)/;
	$InputLocMsg = " [$file line $.]";
	if (/^\$if\s+/) {
	    my($test) = $';
	    push(@level, 'if');
	    if ($action[$#action] ne 'exec') {
		## We're supposed to be skipping or ignoring this level,
		## so for subsequent levels we really ignore completely.
		push(@action, 'ignore');
	    } else {
		## We're executing this IF... do the test.
		## The test is either "term=xxxx", or just a string that
		## we compare to $rl_readline_name;
		if ($test =~ /term=([a-z0-9]+)/) {
		    $test = $1 eq $ENV{'TERM'};
		} else {
		    $test = $test =~ /^(perl|$rl_readline_name)\s*$/i;
		}
		push(@action, $test ? 'exec' : 'skip');
	    }
	    next;
	} elsif (/^\$endif\b/) {
	    die qq/\rWarning$InputLocMsg: unmatched endif\n/ if @level == 0;
	    pop(@level);
	    pop(@action);
	    next;
	} elsif (/^\$else\b/) {
	    die qq/\rWarning$InputLocMsg: unmatched else\n/ if
		@level == 0 || $level[$#level] ne 'if';
	    $level[$#level] = 'else'; ## an IF turns into an ELSE
	    if ($action[$#action] eq 'skip') {
		$action[$#action] = 'exec'; ## if were SKIPing, now EXEC
	    } else {
		$action[$#action] = 'ignore'; ## otherwise, just IGNORE.
	    }
	    next;
	} elsif ($action[$#action] ne 'exec') {
	    ## skipping this one....
	} elsif (m/\s*set\s+(\S+)\s+(\S*)\s*$/) {
	    &rl_set($1, $2, $file);
	} elsif (m/^\s*(\S+):\s+("[^\"]*")\s*$/) {
	    &rl_bind($1, $2);
	} elsif (m/^\s*(\S+):\s+(\S+)\s*$/) {
	    &rl_bind($1, $2);
	} else {
	    chop;
	    warn "\rWarning$InputLocMsg: Bad line [$_]\n" if $^W;
	}
    }
    close(RC);
    ##undef(&F_ReReadInitFile); ## you can do this if you're low on memory
}

sub readline_dumb {
	print $term_OUT $prompt;
	return undef
          if !defined($line = $Term::ReadLine::Perl::term->get_line);
	chop($line);
	$| = $oldbar;
	select $old;
	return $line;
}


##
## This is it. Called as &readline'readline($prompt, $default),
## (DEFAULT can be omitted) the next input line is returned (undef on EOF).
##
sub readline
{
    $Term::ReadLine::Perl::term->register_Tk 
      if not $Term::ReadLine::registered and $Term::ReadLine::toloop
	and defined &Tk::DoOneEvent;
    if ($stdin_not_tty) {
	return undef if !defined($line = <$term_IN>);
	chop($line);
	return $line;
    }

    $old = select $term_OUT;
    $oldbar = $|;
    local($|) = 1;
    local($input);

    ## prompt should be given to us....
    $prompt = defined($_[0]) ? $_[0] : 'INPUT> ';

    if ($dumb_term) {
	return readline_dumb;
    }

    # test if we resume an 'Operate' command
    if ($rl_OperateCount > 0 && (!defined $_[1] || $_[1] eq '')) {
	## it's from a valid previous 'Operate' command and
	## user didn't give a default line
	## we leave $rl_HistoryIndex untouched
	$line = $rl_History[$rl_HistoryIndex];
    } else {
	## set history pointer at the end of history
	$rl_HistoryIndex = $#rl_History + 1;
	$rl_OperateCount = 0;
	$line = defined $_[1] ? $_[1] : '';
    }
    $rl_OperateCount-- if $rl_OperateCount > 0;

    $line_for_revert = $line;

# I don't think we need to do this, actually...
#    while (ioctl(STDIN,$FIONREAD,$fion))
#    {
#	local($n_chars_available) = unpack ($fionread_t, $fion);
#	## print "n_chars = $n_chars_available\n";
#	last if $n_chars_available == 0;
#	$line .= getc_with_pending;  # should we prepend if $rl_start_default_at_beginning?
#    }

    $D = $rl_start_default_at_beginning ? 0 : length($line); ## set dot.
    $LastCommandKilledText = 0;     ## heck, was no last command.
    $lastcommand = '';		    ## Well, there you go.

    ##
    ## some stuff for &redisplay.
    ##
    $lastredisplay = '';	## Was no last redisplay for this time.
    $lastlen = length($lastredisplay);
    $lastdelta = 0;		## Cursor was nowhere
    $si = 0;			## Want line to start left-justified
    $force_redraw = 1;		## Want to display with brute force.
    if (!eval {SetTTY()}) {	## Put into raw mode.
        warn $@ if $@;
        $dumb_term = 1;
	return readline_dumb;
    }
    &redisplay(); 		## Show the line (just prompt at this point).

    *KeyMap = $var_EditingMode;
    undef($AcceptLine);		## When set, will return its value.
    undef($ReturnEOF);		## ...unless this on, then return undef.
    @Pending = ();		## Contains characters to use as input.
    @undo = ();			## Undo history starts empty for each line.

    undef $Vi_undo_state;
    undef $Vi_undo_all_state;

    # We need to do some additional initialization for vi mode.
    &F_ViInput() if $KeyMap{'name'} eq 'vi_keymap';

    # pretend input if we 'Operate' on more than one line
    &F_OperateAndGetNext($rl_OperateCount) if $rl_OperateCount > 0;

    while (!defined($AcceptLine)) {
	## get a character of input
	$input = &getc_with_pending(); # bug in debugger, returns 42. - No more!

	push(@undo, &savestate) unless $Vi_mode; ## save state so we can undo.

	$ThisCommandKilledText = 0;
	##print "\n\rline is @$D:[$line]\n\r"; ##DEBUG
	&do_command($var_EditingMode, 1, ord($input)); ## actually execute input
	*KeyMap = $var_EditingMode;           # JP: added

	# In Vi command mode, don't position the cursor beyond the last
	#     character of the line buffer.
	&F_BackwardChar(1) if $Vi_mode and $line ne ''
	    and &at_end_of_line and $KeyMap{'name'} eq 'vicmd_keymap';

	&redisplay();
	$LastCommandKilledText = $ThisCommandKilledText;
    }

    undef @undo; ## Release the memory.
    &ResetTTY;   ## Restore the tty state.
    $| = $oldbar;
    select $old;
    return undef if defined($ReturnEOF);
    #print STDOUT "|al=`$AcceptLine'";
    $AcceptLine; ## return the line accepted.
}

## ctrl(ord('a')) will return the ordinal for Ctrl-A.
sub ctrl {
  $_[0] ^ (($_[0]>=ord('a') && $_[0]<=ord('z')) ? 0x60 : 0x40);
}



sub SetTTY {
    return if $dumb_term || $stdin_not_tty;
    #return system 'stty raw -echo' if defined &DB::DB;
    if (defined $term_readkey) {
      Term::ReadKey::ReadMode(4, $term_IN);
      return 1;
    }
#   system 'stty raw -echo';

    $sgttyb = ''; ## just to quiet "perl -w";
  if ($useioctl && $^O ne 'solaris' && ioctl($term_IN,$TIOCGETP,$sgttyb)) {
    @tty_buf = unpack($sgttyb_t,$sgttyb);
    if (defined $ENV{OS2_SHELL}) {
      $tty_buf[3] &= ~$mode;
      $tty_buf[3] &= ~$ECHO;
    } else {
      $tty_buf[4] |= $mode;
      $tty_buf[4] &= ~$ECHO;
    }
    $sgttyb = pack($sgttyb_t,@tty_buf);
    ioctl($term_IN,$TIOCSETP,$sgttyb) || die "Can't ioctl TIOCSETP: $!";
  } else {
     warn <<EOW if $useioctl and not defined $ENV{PERL_READLINE_NOWARN};
Can't ioctl TIOCGETP: $!
Consider installing Term::ReadKey from CPAN site nearby
	at http://www.perl.com/CPAN
Or use
	perl -MCPAN -e shell
to reach CPAN. Falling back to 'stty'.
	If you do not want to see this warning, set PERL_READLINE_NOWARN
in your environment.
EOW
					# '; # For Emacs. 
     $useioctl = 0;
     system 'stty raw -echo' and die "Cannot call `stty': $!";
  }
  return 1;
}

sub ResetTTY {
    return if $dumb_term || $stdin_not_tty;
    #return system 'stty -raw echo' if defined &DB::DB;
    if (defined $term_readkey) {
      return Term::ReadKey::ReadMode(0, $term_IN);
    }

#   system 'stty -raw echo';
  if ($useioctl) {
    ioctl($term_IN,$TIOCGETP,$sgttyb) || die "Can't ioctl TIOCGETP: $!";
    @tty_buf = unpack($sgttyb_t,$sgttyb);
    if (defined $ENV{OS2_SHELL}) {
      $tty_buf[3] |= $mode;
      $tty_buf[3] |= $ECHO;
    } else {
      $tty_buf[4] &= ~$mode;
      $tty_buf[4] |= $ECHO;
    }
    $sgttyb = pack($sgttyb_t,@tty_buf);
    ioctl($term_IN,$TIOCSETP,$sgttyb) || die "Can't ioctl TIOCSETP: $!";
  } else {
    system 'stty -raw echo' and die "Cannot call `stty': $!";
  }
}

# Substr_with_props: gives the substr of prompt+string with embedded
# face-change commands

sub substr_with_props {
  my ($p, $s, $from, $len) = @_;
  my $lp = length $p;

  defined $from or $from = 0;
  defined $len or $len = length($p) + length($s) - $from;

  $p =~ s/^(\s*)//; my $bs = $1;
  $p =~ s/(\s*)$//; my $as = $1;

  if ($from >= $lp) {
    return $rl_term_set->[2] . substr($s, $from - $lp, $len) 
      . $rl_term_set->[3];
  } elsif ($from + $len <= $lp) {
    return $bs . $rl_term_set->[0] . substr($p, $from, $len) 
      . $rl_term_set->[1] . $as;
  } else {
    return $bs . $rl_term_set->[0] . substr($p, $from, $lp - $from) 
      . $rl_term_set->[1] . $as
	. $rl_term_set->[2] . substr($s, 0, $from + $len - $lp) 
	  . $rl_term_set->[3];
  }
}

##
## redisplay()
##
## Updates the screen to reflect the current $line.
##
## For the purposes of this routine, we prepend the prompt to a local copy of
## $line so that we display the prompt as well.  We then modify it to reflect
## that some characters have different sizes (i.e. control-C is represented
## as ^C, tabs are expanded, etc.)
##
## This routine is somewhat complicated by two-byte characters.... must
## make sure never to try do display just half of one.
##
## NOTE: If an argument is given, it is used instead of the prompt.
##
## This is some nasty code.
##
sub redisplay
{
    ## local $line has prompt also; take that into account with $D.
    local($prompt) = defined($_[0]) ? $_[0] : $prompt;
    my $oline;
    local($line) = $prompt . $line;
    local($D) = $D + length($prompt);

    ##
    ## If the line contains anything that might require special processing
    ## for displaying (such as tabs, control characters, etc.), we will
    ## take care of that now....
    ##
    if ($line =~ m/[^\x20-\x7e]/)
    {
	local($new, $Dinc, $c) = ('', 0);

	## Look at each character of $line in turn.....
        for ($i = 0; $i < length($line); $i++) {
	    $c = substr($line, $i, 1);

	    ## A tab to expand...
	    if ($c eq "\t") {
		$c = ' ' x  (8 - (($i-length($prompt)) % 8));

	    ## A control character....
	    } elsif ($c =~ tr/\000-\037//) {
		$c = sprintf("^%c", ord($c)+ord('@'));

	    ## the delete character....
	    } elsif (ord($c) == 127) {
		$c = '^?';
	    }
	    $new .= $c;

	    ## Bump over $D if this char is expanded and left of $D.
	    $Dinc += length($c) - 1 if (length($c) > 1 && $i < $D);
	}
	$line = $new;
	$D += $Dinc;
    }

    ##
    ## Now $line is what we'd like to display.
    ##
    ## If it's too long to fit on the line, we must decide what we can fit.
    ##
    ## If we end up moving the screen index ($si) [index of the leftmost
    ## character on the screen], to some place other than the front of the
    ## the line, we'll have to make sure that it's not on the first byte of
    ## a 2-byte character, 'cause we'll be placing a '<' marker there, and
    ## that would screw up the 2-byte character.
    ##
    ## Similarly, if the line needs chopped off, we make sure that the
    ## placement of the tailing '>' won't screw up any 2-byte character in
    ## the vicinity.
    ##
    if ($D == length($prompt)) {
	$si = 0;   ## display from the beginning....
    } elsif ($si >= $D) {
	$si = &max(0, $D - $rl_margin);
	$si-- if $si != length($prompt) && !&OnSecondByte($si);
    } elsif ($si + $rl_screen_width <= $D) {
	$si = &min(length($line), ($D - $rl_screen_width) + $rl_margin);
	$si-- if $si != length($prompt) && !&OnSecondByte($si);
    } else {
	## Fine as-is.... don't need to change $si.
    }
    substr($line, $si, 1) = '<' if $si != 0; ## put the "chopped-off" marker

    $thislen = &min(length($line) - $si, $rl_screen_width);
    if ($si + $thislen < length($line)) {
	## need to place a '>'... make sure to place on first byte.
	$thislen-- if &OnSecondByte($si+$thislen-1);
	substr($line, $si+$thislen-1,1) = '>';
    }

    ##
    ## Now know what to display.
    ## Must get substr($line, $si, $thislen) on the screen,
    ## with the cursor at $D-$si characters from the left edge.
    ##
    $line = substr($line, $si, $thislen);
    $delta = $D - $si;	## delta is cursor distance from left margin.
    if ($si > length($prompt)) {
      $prompt = "";
      $oline = $line;
    } else {
      $oline = substr($line, (length $prompt) - $si);
      $prompt = substr($prompt,$si);
    }

    ##
    ## Now must output $line, with cursor $delta spaces from left margin.
    ##

    ##
    ## If $force_redraw is not set, we can attempt to optimize the redisplay
    ## However, if we don't happen to find an easy way to optimize, we just
    ## fall through to the brute-force method of re-drawing the whole line.
    ##
    if (!$force_redraw)
    {
	## can try to optimize here a bit.

	## For when we only need to move the cursor
	if ($lastredisplay eq $line) {
	    ## If we need to move forward, just overwrite as far as we need.
	    if ($lastdelta < $delta) {
		print $term_OUT (substr_with_props($prompt, $oline, $lastdelta, $delta-$lastdelta));

	    ## Need to move back.
	    } elsif($lastdelta > $delta) {
		## Two ways to move back... use the fastest. One is to just
		## backspace the proper amount. The other is to jump to the
		## the beginning of the line and overwrite from there....
		if ($lastdelta - $delta < $delta) {
		    print $term_OUT "\b" x ($lastdelta - $delta);
		} else {
		    print $term_OUT "\r", substr_with_props($prompt, $oline, 0, $delta);
		}
	    }
	    ($lastlen, $lastredisplay, $lastdelta) = ($thislen, $line, $delta);
	    return;
	}

	## for when we've just added stuff to the end
	if ($thislen > $lastlen &&
	    $lastdelta == $lastlen &&
	    $delta == $thislen &&
	    substr($line, 0, $lastlen) eq $lastredisplay)
	{
	    print $term_OUT (substr_with_props($prompt, $oline, $lastdelta));
	    ($lastlen, $lastredisplay, $lastdelta) = ($thislen, $line, $delta);
	    return;
	}

	## There is much more opportunity for optimizing.....
	## something to work on later.....
    }

    ##
    ## Brute force method of redisplaying... redraw the whole thing.
    ##

    print $term_OUT "\r", substr_with_props($prompt, $oline);
    print $term_OUT ' ' x ($lastlen - $thislen) if $lastlen > $thislen;

    print $term_OUT "\r",substr_with_props($prompt, $oline, 0, $delta)
	if $delta != length ($line) || $lastlen > $thislen;

    ($lastlen, $lastredisplay, $lastdelta) = ($thislen, $line, $delta);

    $force_redraw = 0;
}

sub min     { $_[0] < $_[1] ? $_[0] : $_[1]; }

sub getc_with_pending {

    my $key = @Pending ? shift(@Pending) : &$rl_getc;

    # Save keystrokes for vi '.' command
    push(@$Dot_buf, $key) if $Dot_buf;

    $key;
}

sub rl_getc {
	  my $key;                        # JP: Added missing declaration
	  if (defined $term_readkey) { # XXXX ???
	    $Term::ReadLine::Perl::term->Tk_loop 
	      if $Term::ReadLine::toloop && defined &Tk::DoOneEvent;
	    $key = Term::ReadKey::ReadKey(0, $term_IN);
	  } else {
	    $key = $Term::ReadLine::Perl::term->get_c;
	  }
}

##
## do_command(keymap, numericarg, command)
##
## If the KEYMAP has an entry for COMMAND, it is executed.
## Otherwise, the default command for the keymap is executed.
##
sub do_command
{
    local *KeyMap = shift;
    my ($count, $key) = @_;
    my $cmd = defined($KeyMap[$key]) ? $KeyMap[$key]
                                     : ($KeyMap{'default'} || 'F_Ding');
    if (!defined($cmd) || $cmd eq ''){
	warn "internal error (key=$key)";
    } else {
	## print "COMMAND [$cmd($count, $key)]\r\n"; ##DEBUG
	&$cmd($count, $key);
    }
    $lastcommand = $cmd;
}

##
## Save whatever state we wish to save as an anonymous array.
## The only other function that needs to know about its encoding is getstate.
##
sub savestate
{
    [$D, $si, $LastCommandKilledText, $KillBuffer, $line];
}


##
## $_[1] is an ASCII ordinal; inserts as per $count.
##
sub F_SelfInsert
{
    my ($count, $ord) = @_;
    my $text2add = pack('c', $ord) x $count;
    if ($InsertMode) {
	substr($line,$D,0) .= $text2add;
    } else {
	## note: this can screw up with 2-byte characters.
	substr($line,$D,length($text2add)) = $text2add;
    }
    $D += length($text2add);
}

##
## Return the line as-is to the user.
##
sub F_AcceptLine
{
    &add_line_to_history;
    $AcceptLine = $line;
    print $term_OUT "\r\n";
}

sub add_line_to_history
{
    ## Insert into history list if:
    ##	 * bigger than the minimal length
    ##   * not same as last entry
    ##
    if (length($line) >= $minlength 
	&& (!@rl_History || $rl_History[$#rl_History] ne $line)
       ) {
	## if the history list is full, shift out an old one first....
	while (@rl_History >= $rl_MaxHistorySize) {
	    shift(@rl_History);
	    $rl_HistoryIndex--;
	}

	push(@rl_History, $line); ## tack new one on the end
    }
}

#sub F_ReReadInitFile;
#sub rl_getc;
sub F_ForwardChar;
sub F_BackwardChar;
sub F_BeginningOfLine;
sub F_EndOfLine;
sub F_ForwardWord;
sub F_BackwardWord;
sub F_RedrawCurrentLine;
sub F_ClearScreen;
# sub F_SelfInsert;
sub F_QuotedInsert;
sub F_TabInsert;
#sub F_AcceptLine;
sub F_OperateAndGetNext;
sub F_BackwardDeleteChar;
sub F_DeleteChar;
sub F_UnixWordRubout;
sub F_UnixLineDiscard;
sub F_UpcaseWord;
sub F_DownCaseWord;
sub F_CapitalizeWord;
sub F_TransposeWords;
sub F_TransposeChars;
sub F_PreviousHistory;
sub F_NextHistory;
sub F_BeginningOfHistory;
sub F_EndOfHistory;
sub F_ReverseSearchHistory;
sub F_ForwardSearchHistory;
sub F_HistorySearchBackward;
sub F_HistorySearchForward;
sub F_KillLine;
sub F_BackwardKillLine;
sub F_Yank;
sub F_YankPop;
sub F_YankNthArg;
sub F_KillWord;
sub F_BackwardKillWord;
sub F_Abort;
sub F_DoLowercaseVersion;
sub F_Undo;
sub F_RevertLine;
sub F_EmacsEditingMode;
sub F_Interrupt;
sub F_PrefixMeta;
sub F_UniversalArgument;
sub F_DigitArgument;
sub F_OverwriteMode;
sub F_InsertMode;
sub F_ToggleInsertMode;
sub F_Suspend;
sub F_Ding;
sub F_PossibleCompletions;
sub F_Complete;

# Comment next line and __DATA__ line below to disable the selfloader.

use SelfLoader;

1;

__DATA__

# From here on anything may be autoloaded

sub max     { $_[0] > $_[1] ? $_[0] : $_[1]; }
sub isupper { ord($_[0]) >= ord('A') && ord($_[0]) <= ord('Z'); }
sub islower { ord($_[0]) >= ord('a') && ord($_[0]) <= ord('z'); }
sub toupper { &islower ? pack('c', ord($_[0])-ord('a')+ord('A')) : $_[0];}
sub tolower { &isupper ? pack('c', ord($_[0])-ord('A')+ord('a')) : $_[0];}

##
## rl_set(var_name, value_string)
##
## Sets the named variable as per the given value, if both are appropriate.
## Allows the user of the package to set such things as HorizontalScrollMode
## and EditingMode.  Value_string may be of the form
##	HorizontalScrollMode
##      horizontal-scroll-mode
##
## Also called during the parsing of ~/.inputrc for "set var value" lines.
##
## The previous value is returned, or undef on error.
###########################################################################
## Consider the following example for how to add additional variables
## accessible via rl_set (and hence via ~/.inputrc).
##
## Want:
## We want an external variable called "FooTime" (or "foo-time").
## It may have values "January", "Monday", or "Noon".
## Internally, we'll want those values to translate to 1, 2, and 12.
##
## How:
## Have an internal variable $var_FooTime that will represent the current
## internal value, and initialize it to the default value.
## Make an array %var_FooTime whose keys and values are are the external
## (January, Monday, Noon) and internal (1, 2, 12) values:
##
##	    $var_FooTime = $var_FooTime{'January'} =  1; #default
##	                   $var_FooTime{'Monday'}  =  2;
##	                   $var_FooTime{'Noon'}    = 12;
##
sub rl_set
{
    local($var, $val) = @_;

    $var = 'CompleteAddsuffix' if $var eq 'visible-stats';

    ## if the variable is in the form "some-name", change to "SomeName"
    local($_) = "\u$var";
    local($return) = undef;
    s/-(.)/\u$1/g;

    local(*V) = $ {'readline::'}{"var_$_"};
    if (!defined($V)) {
	warn("Warning$InputLocMsg:\n".
	     "  Invalid variable `$var'\n") if $^W;
    } elsif (!defined($V{$val})) {
	local(@selections) = keys(%V);
	warn("Warning$InputLocMsg:\n".
	     "  Invalid value `$val' for variable `$var'.\n".
	     "  Choose from [@selections].\n") if $^W;
    } else {
	$return = $V;
        $V = $V{$val}; ## make the setting
    }
    $return;
}

##
## OnSecondByte($index)
##
## Returns true if the byte at $index into $line is the second byte
## of a two-byte character.
##
sub OnSecondByte
{
    return 0 if $_[0] == 0 || $_[0] == length($line);

    die 'internal error' if $_[0] > length($line);

    ##
    ## must start looking from the beginning of the line .... can
    ## have one- and two-byte characters interspersed, so can't tell
    ## without starting from some know location.....
    ##
    local($i);
    for ($i = 0; $i < $_[0]; $i++) {
	next if ord(substr($line, $i, 1)) < 0x80;
	## We have the first byte... must bump up $i to skip past the 2nd.
	## If that one we're skipping past is the index, it should be changed
	## to point to the first byte of the pair (therefore, decremented).
        return 1 if ++$i == $_[0];
    }
    0; ## seemed to be OK.
}

##
## CharSize(index)
##
## Returns the size of the character at the given INDEX in the
## current line.  Most characters are just one byte in length,
## but if the byte at the index and the one after has the high
## bit set those two bytes are one character of size=2.
##
## Assumes that index points to the first of a 2-byte char if not
## pointing to a 2-byte char.
##
sub CharSize
{
    return 2 if ord(substr($line, $_[0],   1)) >= 0x80 &&
                ord(substr($line, $_[0]+1, 1)) >= 0x80;
    1;
}

sub GetTTY
{
    $base_termios = $termios;  # make it long enough
    ioctl($term_IN,$TCGETS,$base_termios) || die "Can't ioctl TCGETS: $!";
}

sub XonTTY
{
    # I don't know which of these I actually need to do this to, so we'll
    # just cover all bases.

    ioctl($term_IN,$TCXONC,$TCOON);    # || die "Can't ioctl TCXONC STDIN: $!";
    ioctl($term_OUT,$TCXONC,$TCOON);   # || die "Can't ioctl TCXONC STDOUT: $!";
}

sub ___SetTTY
{
# print "before SetTTY\n\r";
# system 'stty -a';

    &XonTTY;

    &GetTTY
	if !defined($base_termios);

    @termios = unpack($termios_t,$base_termios);
    $termios[$TERMIOS_IFLAG] |= $TERMIOS_READLINE_ION;
    $termios[$TERMIOS_IFLAG] &= ~$TERMIOS_READLINE_IOFF;
    $termios[$TERMIOS_OFLAG] |= $TERMIOS_READLINE_OON;
    $termios[$TERMIOS_OFLAG] &= ~$TERMIOS_READLINE_OOFF;
    $termios[$TERMIOS_LFLAG] |= $TERMIOS_READLINE_LON;
    $termios[$TERMIOS_LFLAG] &= ~$TERMIOS_READLINE_LOFF;
    $termios[$TERMIOS_VMIN] = 1;
    $termios[$TERMIOS_VTIME] = 0;
    $termios = pack($termios_t,@termios);
    ioctl($term_IN,$TCSETS,$termios) || die "Can't ioctl TCSETS: $!";

# print "after SetTTY\n\r";
# system 'stty -a';
}

sub normal_tty_mode
{
    return if $stdin_not_tty || $dumb_term || !$initialized;
    &XonTTY;
    &GetTTY if !defined($base_termios);
    &ResetTTY;
}

sub ___ResetTTY
{
# print "before ResetTTY\n\r";
# system 'stty -a';

    @termios = unpack($termios_t,$base_termios);
    $termios[$TERMIOS_IFLAG] |= $TERMIOS_NORMAL_ION;
    $termios[$TERMIOS_IFLAG] &= ~$TERMIOS_NORMAL_IOFF;
    $termios[$TERMIOS_OFLAG] |= $TERMIOS_NORMAL_OON;
    $termios[$TERMIOS_OFLAG] &= ~$TERMIOS_NORMAL_OOFF;
    $termios[$TERMIOS_LFLAG] |= $TERMIOS_NORMAL_LON;
    $termios[$TERMIOS_LFLAG] &= ~$TERMIOS_NORMAL_LOFF;
    $termios = pack($termios_t,@termios);
    ioctl($term_IN,$TCSETS,$termios) || die "Can't ioctl TCSETS: $!";

# print "after ResetTTY\n\r";
# system 'stty -a';
}

##
## WordBreak(index)
##
## Returns true if the character at INDEX into $line is a basic word break
## character, false otherwise.
##
sub WordBreak
{
    index($rl_basic_word_break_characters, substr($line,$_[0],1)) != -1;
}

sub getstate
{
    ($D, $si, $LastCommandKilledText, $KillBuffer, $line) = @{$_[0]};
    $ThisCommandKilledText = $LastCommandKilledText;
}

##
## kills from D=$_[0] to $_[1] (to the killbuffer if $_[2] is true)
##
sub kill_text
{
    my($from, $to, $save) = (&min($_[0], $_[1]), &max($_[0], $_[1]), $_[2]);
    my $len = $to - $from;
    if ($save) {
	$ThisCommandKilledText = 1;
	$KillBuffer = '' if !$LastCommandKilledText;
	$KillBuffer .= substr($line, $from, $len);
    }
    substr($line, $from, $len) = '';

    ## adjust $D
    if ($D > $from) {
	$D -= $len;
	$D = $from if $D < $from;
    }
}


###########################################################################
## Bindable functions... pretty much in the same order as in readline.c ###
###########################################################################

##
## Returns true if $D at the end of the line.
##
sub at_end_of_line
{
    ($D + &CharSize($D)) == (length($line) + 1);
}


##
## Move forward (right) $count characters.
##
sub F_ForwardChar
{
    my $count = shift;
    return &F_BackwardChar(-$count) if $count < 0;

    while (!&at_end_of_line && $count-- > 0) {
	$D += &CharSize($D);
    }
}

##
## Move backward (left) $count characters.
##
sub F_BackwardChar
{
    my $count = shift;
    return &F_ForwardChar(-$count) if $count < 0;

    while (($D > 0) && ($count-- > 0)) {
	$D--;  		           ## Move back one regardless,
	$D-- if &OnSecondByte($D); ## another if over a big char.
    }
}

##
## Go to beginning of line.
##
sub F_BeginningOfLine
{
    $D = 0;
}

##
## Move to the end of the line.
##
sub F_EndOfLine
{
    &F_ForwardChar(100) while !&at_end_of_line;
}

##
## Move to the end of this/next word.
## Done as many times as $count says.
##
sub F_ForwardWord
{
    my $count = shift;
    return &F_BackwardWord(-$count) if $count < 0;

    while (!&at_end_of_line && $count-- > 0)
    {
	## skip forward to the next word (if not already on one)
	&F_ForwardChar(1) while !&at_end_of_line && &WordBreak($D);
	## skip forward to end of word
	&F_ForwardChar(1) while !&at_end_of_line && !&WordBreak($D);
    }
}

##
## 
## Move to the beginning of this/next word.
## Done as many times as $count says.
##
sub F_BackwardWord
{
    my $count = shift;
    return &F_ForwardWord(-$count) if $count < 0;

    while ($D > 0 && $count-- > 0) {
	## skip backward to the next word (if not already on one)
	&F_BackwardChar(1) while (($D > 0) && &WordBreak($D-1));
	## skip backward to start of word
	&F_BackwardChar(1) while (($D > 0) && !&WordBreak($D-1));
    }
}

##
## Refresh the input line.
##
sub F_RedrawCurrentLine
{
    $force_redraw = 1;
}

##
## Clear the screen and refresh the line.
## If given a numeric arg other than 1, simply refreshes the line.
##
sub F_ClearScreen
{
    my $count = shift;
    return &F_RedrawCurrentLine if $count != 1;

    $rl_CLEAR = `clear` if !defined($rl_CLEAR);
    print $term_OUT $rl_CLEAR;
    $force_redraw = 1;
}

##
## Insert the next character read verbatim.
##
sub F_QuotedInsert
{
    my $count = shift;
    &F_SelfInsert($count, ord(&getc_with_pending));
}

##
## Insert a tab.
##
sub F_TabInsert
{
    my $count = shift;
    &F_SelfInsert($count, ord("\t"));
}

## Operate - accept the current line and fetch from the
## history the next line relative to current line for default.
sub F_OperateAndGetNext
{
    my $count = shift;

    &F_AcceptLine;

    my $remainingEntries = $#rl_History - $rl_HistoryIndex;
    if ($count > 0 && $remainingEntries >= 0) {  # there is something to repeat
	if ($remainingEntries > 0) {  # if we are not on last line
	    $rl_HistoryIndex++;       # fetch next one
	    $count = $remainingEntries if $count > $remainingEntries;
	}
	$rl_OperateCount = $count;
    }
}

##
## Removes $count chars to left of cursor (if not at beginning of line).
## If $count > 1, deleted chars saved to kill buffer.
##
sub F_BackwardDeleteChar
{
    my $count = shift;
    return F_DeleteChar(-$count) if $count < 0;
    my $oldD = $D;
    &F_BackwardChar($count);
    return if $D == $oldD;
    &kill_text($oldD, $D, $count > 1);
}

##
## Removes the $count chars from under the cursor.
## If there is no line and the last command was different, tells
## readline to return EOF.
## If there is a line, and the cursor is at the end of it, and we're in
## tcsh completion mode, then list possible completions.
## If $count > 1, deleted chars saved to kill buffer.
##
sub F_DeleteChar
{
    my $count = shift;
    return F_DeleteBackwardChar(-$count) if $count < 0;
    if (length($line) == 0) {	# EOF sent (probably OK in DOS too)
	$AcceptLine = $ReturnEOF = 1 if $lastcommand ne 'F_DeleteChar';
	return;
    }
    if ($D == length ($line))
    {
	&complete_internal('?') if $var_TcshCompleteMode;
	return;
    }
    my $oldD = $D;
    &F_ForwardChar($count);
    return if $D == $oldD;
    &kill_text($oldD, $D, $count > 1);
}

##
## Kill to previous whitespace.
##
sub F_UnixWordRubout
{
    return &F_Ding if $D == 0;
    (my $oldD, local $rl_basic_word_break_characters) = ($D, "\t ");
			     # JP:  Fixed a bug here - both were 'my'
    F_BackwardWord(1);
    kill_text($D, $oldD, 1);
}

##
## Kill line from cursor to beginning of line.
##
sub F_UnixLineDiscard
{
    return &F_Ding if $D == 0;
    kill_text(0, $D, 1);
}

sub F_UpcaseWord     { &changecase($_[0], 'up');   }
sub F_DownCaseWord   { &changecase($_[0], 'down'); }
sub F_CapitalizeWord { &changecase($_[0], 'cap');  }

##
## Translated from GNUs readline.c
## One arg is 'up' to upcase $_[0] words,
##            'down' to downcase them,
##         or something else to capitolize them.
## If $_[0] is negative, the dot is not moved.
##
sub changecase
{
    my $op = $_[1];

    my ($start, $state, $c, $olddot) = ($D, 0);
    if ($_[0] < 0)
    {
	$olddot = $D;
	$_[0] = -$_[0];
    }

    &F_ForwardWord;  ## goes forward $_[0] words.

    while ($start < $D) {
	$c = substr($line, $start, 1);

	if ($op eq 'up') {
	    $c = &toupper($c);
	} elsif ($op eq 'down') {
	    $c = &tolower($c);
	} else { ## must be 'cap'
	    if ($state == 1) {
	        $c = &tolower($c);
	    } else {
	        $c = &toupper($c);
		$state = 1;
	    }
	    $state = 0 if $c !~ tr/a-zA-Z//;
	}

	substr($line, $start, 1) = $c;
	$start++;
    }
    $D = $olddot if defined($olddot);
}

sub F_TransposeWords { } ## not implemented yet

##
## Switch char at dot with char before it.
## If at the end of the line, switch the previous two...
## (NOTE: this could screw up multibyte characters.. should do correctly)
sub F_TransposeChars
{
    if ($D == length($line) && $D >= 2) {
        substr($line,$D-2,2) = substr($line,$D-1,1).substr($line,$D-2,1);
    } elsif ($D >= 1) {
	substr($line,$D-1,2) = substr($line,$D,1)  .substr($line,$D-1,1);
    } else {
	&F_Ding;
    }
}

##
## Use the previous entry in the history buffer (if there is one)
##
sub F_PreviousHistory
{
    return if $rl_HistoryIndex == 0;

    $rl_HistoryIndex--;
    ($D, $line) = (0, $rl_History[$rl_HistoryIndex]);
    &F_EndOfLine;
}

##
## Use the next entry in the history buffer (if there is one)
##
sub F_NextHistory
{
    return if $rl_HistoryIndex > $#rl_History;

    $rl_HistoryIndex++;
    if ($rl_HistoryIndex > $#rl_History) {
	$D = 0;
	$line = '';
    } else {
	($D, $line) = (0, $rl_History[$rl_HistoryIndex]);
	&F_EndOfLine;
    }
}

sub F_BeginningOfHistory
{
    if ($rl_HistoryIndex != 0) {
	$rl_HistoryIndex = 0;
	($D, $line) = (0, $rl_History[$rl_HistoryIndex]);
	&F_EndOfLine;
    }
}

sub F_EndOfHistory
{
    if (@rl_History != 0 && $rl_HistoryIndex != $#rl_History) {
	$rl_HistoryIndex = $#rl_History;
	($D, $line) = (0, $rl_History[$rl_HistoryIndex]);
	&F_EndOfLine;
    }
}

sub F_ReverseSearchHistory
{
    &DoSearch($_[0] >= 0 ? 1 : 0);
}

sub F_ForwardSearchHistory
{
    &DoSearch($_[0] >= 0 ? 0 : 1);
}

sub F_HistorySearchBackward
{
    &DoSearchStart(($_[0] >= 0 ? 1 : 0),substr($line,0,$D));
}

sub F_HistorySearchForward
{
    &DoSearchStart(($_[0] >= 0 ? 0 : 1),substr($line,0,$D));
}

## returns a new $i or -1 if not found.
sub search { 
  my ($i, $str) = @_;
  return -1 if $i < 0 || $i > $#rl_History; 	 ## for safety
  while (1) {
    return $i if rindex($rl_History[$i], $str) >= 0;
    if ($reverse) {
      return -1 if $i-- == 0;
    } else {
      return -1 if $i++ == $#rl_History;
    }
  }
}

sub DoSearch
{
    local $reverse = shift;	# Used in search()
    my $oldline = $line;
    my $oldD = $D;

    my $searchstr = '';  ## string we're searching for
    my $I = -1;  	     ## which history line

    $si = 0;

    while (1)
    {
	if ($I != -1) {
	    $line = $rl_History[$I];
	    $D += index($rl_History[$I], $searchstr);
	}
	&redisplay( '('.($reverse?'reverse-':'') ."i-search) `$searchstr': ");

	$c = &getc_with_pending;
	if ($KeyMap[ord($c)] eq 'F_ReverseSearchHistory') {
	    if ($reverse && $I != -1) {
		if ($tmp = &search($I-1,$searchstr), $tmp >= 0) {
		    $I = $tmp;
		} else {
		    &F_Ding;
		}
	    }
	    $reverse = 1;
	} elsif ($KeyMap[ord($c)] eq 'F_ForwardSearchHistory') {
	    if (!$reverse && $I != -1) {
		if ($tmp = &search($I+1,$searchstr), $tmp >= 0) {
		    $I = $tmp;
		} else {
		    &F_Ding;
		}
	    }
	    $reverse = 0;
        } elsif ($c eq "\007") {  ## abort search... restore line and return
	    $line = $oldline;
	    $D = $oldD;
	    return;
        } elsif (ord($c) < 32 || ord($c) > 126) {
	    push(@Pending, $c) if $c ne "\e";
	    if ($I < 0) {
		## just restore
		$line = $oldline;
		$D = $oldD;
	    } else {
		#chose this line
		$line = $rl_History[$I];
		$D = index($rl_History[$I], $searchstr);
	    }
	    &redisplay();
	    last;
	} else {
	    ## Add this character to the end of the search string and
	    ## see if that'll match anything.
	    $tmp = &search($I < 0 ? $rl_HistoryIndex-$reverse: $I, $searchstr.$c);
	    if ($tmp == -1) {
		&F_Ding;
	    } else {
		$searchstr .= $c;
		$I = $tmp;
	    }
	}
    }
}

## returns a new $i or -1 if not found.
sub searchStart { 
  my ($i, $reverse, $str) = @_;
  $i += $reverse ? - 1: +1;
  return -1 if $i < 0 || $i > $#rl_History;  ## for safety
  while (1) {
    return $i if index($rl_History[$i], $str) == 0;
    if ($reverse) {
      return -1 if $i-- == 0;
    } else {
      return -1 if $i++ == $#rl_History;
    }
  }
}

sub DoSearchStart
{
    my ($reverse,$what) = @_;
    my $i = searchStart($rl_HistoryIndex, $reverse, $what);
    return if $i == -1;
    $rl_HistoryIndex = $i;
    ($D, $line) = (0, $rl_History[$rl_HistoryIndex]);
    F_BeginningOfLine();
    F_ForwardChar(length($what));

}

###########################################################################
###########################################################################

##
## Kill from cursor to end of line.
##
sub F_KillLine
{
    my $count = shift;
    return F_BackwardKillLine(-$count) if $count < 0;
    kill_text($D, length($line), 1);
}

##
## Delete from cursor to beginning of line.
##
sub F_BackwardKillLine
{
    my $count = shift;
    return F_KillLine(-$count) if $count < 0;
    return F_Ding if $D == 0;
    kill_text(0, $D, 1);
}

##
## TextInsert(count, string)
##
sub TextInsert {
  my $count = shift;
  my $text2add = shift(@_) x $count;
  if ($InsertMode) {
    substr($line,$D,0) .= $text2add;
  } else {
    substr($line,$D,length($text2add)) = $text2add;
  }
  $D += length($text2add);
}

sub F_Yank
{
    &TextInsert($_[0], $KillBuffer);
}

sub F_YankPop    { } ## not implemented yet
sub F_YankNthArg { } ## not implemented yet

##
## Kill to the end of the current word. If not on a word, kill to
## the end of the next word.
##
sub F_KillWord
{
    my $count = shift;
    return &F_BackwardKillWord(-$count) if $count < 0;
    my $oldD = $D;
    &F_ForwardWord($count);	## moves forward $count words.
    kill_text($oldD, $D, 1);
}

##
## Kill backward to the start of the current word, or, if currently
## not on a word (or just at the start of a word), to the start of the
## previous word.
##
sub F_BackwardKillWord
{
    my $count = shift;
    return F_KillWord(-$count) if $count < 0;
    my $oldD = $D;
    &F_BackwardWord($count);	## moves backward $count words.
    kill_text($D, $oldD, 1);
}

###########################################################################
###########################################################################


##
## Abort the current input.
##
sub F_Abort
{
    &F_Ding;
}


##
## If the character that got us here is upper case,
## do the lower-case equiv...
##
sub F_DoLowercaseVersion
{
    if ($_[1] >= ord('A') && $_[1] <= ord('Z')) {
	&do_command(*KeyMap, $_[0], $_[1] - ord('A') + ord('a'));
    } else {
	&F_Ding;
    }
}

##
## Undo one level.
##
sub F_Undo
{
    pop(@undo); ## get rid of the state we just put on, so we can go back one.
    if (@undo) {
	&getstate(pop(@undo));
    } else {
	&F_Ding;
    }
}

##
## Replace the current line to some "before" state.
##
sub F_RevertLine
{
    if ($rl_HistoryIndex >= $#rl_History+1) {
	$line = $line_for_revert;
    } else {
	$line = $rl_History[$rl_HistoryIndex];
    }
    $D = length($line);
}

sub F_EmacsEditingMode
{
    $var_EditingMode = $var_EditingMode{'emacs'};
    $Vi_mode = 0;
}

###########################################################################
###########################################################################


##
## (Attempt to) interrupt the current program.
##
sub F_Interrupt
{
    print $term_OUT "\r\n";
    &ResetTTY;
    kill ("INT", 0);

    ## We're back.... must not have died.
    $force_redraw = 1;
}

##
## Execute the next character input as a command in a meta keymap.
##
sub F_PrefixMeta
{
    my($count, $keymap) = ($_[0], "$KeyMap{'name'}_$_[1]");
    ##print "F_PrefixMeta [$keymap]\n\r";
    die "<internal error, $_[1]>" unless %$keymap;
    do_command(*$keymap, $count, ord(&getc_with_pending));
}

sub F_UniversalArgument
{
    &F_DigitArgument;
}

##
## For typing a numeric prefix to a command....
##
sub F_DigitArgument
{
    my $ord = $_[1];
    my ($NumericArg, $sign, $explicit) = (1, 1, 0);
    my $increment;

    do
    {
	if (defined($KeyMap[$ord]) && $KeyMap[$ord] eq 'F_UniversalArgument') {
	    $NumericArg *= 4;
	} elsif ($ord == ord('-') && !$explicit) {
	    $sign = -$sign;
	    $NumericArg = $sign;
	} elsif ($ord >= ord('0') && $ord <= ord('9')) {
	    $increment = ($ord - ord('0')) * $sign;
	    if ($explicit) {
		$NumericArg = $NumericArg * 10 + $increment;
	    } else {
		$NumericArg = $increment;
		$explicit = 1;
	    }
	} else {
	    local(*KeyMap) = $var_EditingMode;
	    &redisplay();
	    &do_command(*KeyMap, $NumericArg, $ord);
	    return;
	}
	## make sure it's not toooo big.
	if ($NumericArg > $rl_max_numeric_arg) {
	    $NumericArg = $rl_max_numeric_arg;
	} elsif ($NumericArg < -$rl_max_numeric_arg) {
	    $NumericArg = -$rl_max_numeric_arg;
	}
	&redisplay(sprintf("(arg %d) ", $NumericArg));
    } while $ord = ord(&getc_with_pending);
}

sub F_OverwriteMode
{
    $InsertMode = 0;
}

sub F_InsertMode
{
    $InsertMode = 1;
}

sub F_ToggleInsertMode
{
    $InsertMode = !$InsertMode;
}

##
## (Attempt to) suspend the program.
##
sub F_Suspend
{
    if ($inDOS && length($line)==0) { # EOF sent
	$AcceptLine = $ReturnEOF = 1 if $lastcommand ne 'F_DeleteChar';
	return;
    }
    print $term_OUT "\r\n";
    &ResetTTY;
    eval { kill ("TSTP", 0) };
    ## We're back....
    &SetTTY;
    $force_redraw = 1;
}

##
## Ring the bell.
## Should do something with $var_PreferVisibleBell here, but what?
##
sub F_Ding {
    print $term_OUT "\007";
    return;    # Undefined return value
}

##########################################################################
#### command/file completion  ############################################
##########################################################################

##
## How Command Completion Works
##
## When asked to do a completion operation, readline isolates the word
## to the immediate left of the cursor (i.e. what's just been typed).
## This information is then passed to some function (which may be supplied
## by the user of this package) which will return an array of possible
## completions.
##
## If there is just one, that one is used.  Otherwise, they are listed
## in some way (depends upon $var_TcshCompleteMode).
##
## The default is to do filename completion.  The function that performs
## this task is readline'rl_filename_list.
##
## A minimal-trouble way to have command-completion is to call
## readline'rl_basic_commands with an array of command names, such as
##    &readline'rl_basic_commands('quit', 'run', 'set', 'list')
## Those command names will then be used for completion if the word being
## completed begins the line. Otherwise, completion is disallowed.
##
## The way to have the most power is to provide a function to readline
## which will accept information about a partial word that needs completed,
## and will return the appropriate list of possibilities.
## This is done by setting $readline'rl_completion_function to the name of
## the function to run.
##
## That function will be called with three args ($text, $line, $start).
## TEXT is the partial word that should be completed.  LINE is the entire
## input line as it stands, and START is the index of the TEXT in LINE
## (i.e. zero if TEXT is at the beginning of LINE).
##
## A cool completion function will look at LINE and START and give context-
## sensitive completion lists. Consider something that will do completion
## for two commands
## 	cat FILENAME
##	finger USERNAME
##	status [this|that|other]
##
## It (untested) might look like:
##
##	$readline'rl_completion_function = "main'complete";
##	sub complete { local($text, $_, $start) = @_;
##	    ## return commands which may match if at the beginning....
##	    return grep(/^$text/, 'cat', 'finger') if $start == 0;
##	    return &rl_filename_list($text) if /^cat\b/;
##	    return &my_namelist($text) if /^finger\b/;
##	    return grep(/^text/, 'this', 'that','other') if /^status\b/;
##	    ();
##	}
## Of course, a real completion function would be more robust, but you
## get the idea (I hope).
##

##
## List possible completions
##
sub F_PossibleCompletions
{
    &complete_internal('?');
}

##
## List possible completions
##
sub F_InsertPossibleCompletions
{
    &complete_internal('*');
}

##
## Do a completion operation.
## If the last thing we did was a completion operation, we'll
## now list the options available (under normal emacs mode).
##
## Under TcshCompleteMode, each contiguous subsequent completion operation
## lists another of the possible options.
##
## Returns true if a completion was done, false otherwise, so vi completion
##     routines can test it.
##
sub F_Complete
{
    if ($lastcommand eq 'F_Complete') {
	if ($var_TcshCompleteMode && @tcsh_complete_selections > 0) {
	    substr($line, $tcsh_complete_start, $tcsh_complete_len)
		= $tcsh_complete_selections[0];
	    $D -= $tcsh_complete_len;
	    $tcsh_complete_len = length($tcsh_complete_selections[0]);
	    $D += $tcsh_complete_len;
	    push(@tcsh_complete_selections, shift(@tcsh_complete_selections));
	} else {
	    &complete_internal('?') or return;
	}
    } else {
	@tcsh_complete_selections = ();
	&complete_internal("\t") or return;
    }

    1;
}

##
## The meat of command completion. Patterned closely after GNU's.
##
## The supposedly partial word at the cursor is "completed" as per the
## single argument:
##	"\t"	complete as much of the word as is unambiguous
##	"?"	list possibilities.
## 	"*"	replace word with all possibilities. (who would use this?)
##
## A few notable variables used:
##   $rl_completer_word_break_characters
##	-- characters in this string break a word.
##   $rl_special_prefixes
##	-- but if in this string as well, remain part of that word.
##
## Returns true if a completion was done, false otherwise, so vi completion
##     routines can test it.
##
sub complete_internal
{
    my $what_to_do = shift;
    my ($point, $end) = ($D, $D);

    # In vi mode, complete if the cursor is at the *end* of a word, not
    #     after it.
    ($point++, $end++) if $Vi_mode;

    if ($point)
    {
        ## Not at the beginning of the line; Isolate the word to be completed.
	1 while (--$point && (-1 == index($rl_completer_word_break_characters,
		substr($line, $point, 1))));

	# Either at beginning of line or at a word break.
	# If at a word break (that we don't want to save), skip it.
	$point++ if (
    		(index($rl_completer_word_break_characters,
		       substr($line, $point, 1)) != -1) &&
    		(index($rl_special_prefixes, substr($line, $point, 1)) == -1)
	);
    }

    my $text = substr($line, $point, $end - $point);
    $rl_completer_terminator_character = ' ';
    @matches = &completion_matches($rl_completion_function,$text,$line,$point);

    if (@matches == 0) {
	return &F_Ding;
    } elsif ($what_to_do eq "\t") {
	my $replacement = shift(@matches);
	$replacement .= $rl_completer_terminator_character if @matches == 1;
	&F_Ding if @matches != 1;
	if ($var_TcshCompleteMode) {
	    @tcsh_complete_selections = (@matches, $text);
	    $tcsh_complete_start = $point;
	    $tcsh_complete_len = length($replacement);
	}
	if ($replacement ne '') {
	    substr($line, $point, $end-$point) = $replacement;
	    $D = $D - ($end - $point) + length($replacement);
	}
    } elsif ($what_to_do eq '?') {
	shift(@matches); ## remove prepended common prefix
	print $term_OUT "\n\r";
	# print "@matches\n\r";
	&pretty_print_list (@matches);
	$force_redraw = 1;
    } elsif ($what_to_do eq '*') {
	shift(@matches); ## remove common prefix.
	local $" = $rl_completer_terminator_character;
	my $replacement = "@matches$rl_completer_terminator_character";
	substr($line, $point, $end-$point) = $replacement; ## insert all.
	$D = $D - ($end - $point) + length($replacement);
    } else {
	warn "\r\n[Internal error]";
	return &F_Ding;
    }

    1;
}

##
## completion_matches(func, text, line, start)
##
## FUNC is a function to call as FUNC(TEXT, LINE, START)
## 	where TEXT is the item to be completed
##	      LINE is the whole command line, and
##	      START is the starting index of TEXT in LINE.
## The FUNC should return a list of items that might match.
##
## completion_matches will return that list, with the longest common
## prefix prepended as the first item of the list.  Therefor, the list
## will either be of zero length (meaning no matches) or of 2 or more.....
##

## Works with &rl_basic_commands. Return items from @rl_basic_commands
## that start with the pattern in $text.
sub use_basic_commands {
  my ($text, $line, $start) = @_;
  return () if $start != 0;
  grep(/^$text/, @rl_basic_commands);
}

sub completion_matches
{
    my ($func, $text, $line, $start) = @_;

    ## get the raw list
    my @matches;

    #print qq/\r\neval("\@matches = &$func(\$text, \$line, \$start)\n\r/;#DEBUG
    #eval("\@matches = &$func(\$text, \$line, \$start);1") || warn "$@ ";
    @matches = &$func($text, $line, $start);

    ## if anything returned , find the common prefix among them
    if (@matches) {
	my $prefix = $matches[0];
	my $len = length($prefix);
	for ($i = 1; $i < @matches; $i++) {
	    next if substr($matches[$i], 0, $len) eq $prefix;
	    $prefix = substr($prefix, 0, --$len);
	    last if $len == 0;
	    $i--; ## retry this one to see if the shorter one matches.
	}
	unshift(@matches, $prefix); ## make common prefix the first thing.
    }
    @matches;
}

##
## For use in passing to completion_matches(), returns a list of
## filenames that begin with the given pattern.  The user of this package
## can set $rl_completion_function to 'rl_filename_list' to restore the
## default of filename matching if they'd changed it earlier, either
## directly or via &rl_basic_commands.
##
sub rl_filename_list
{
    my $pattern = $_[0];
    my @files = (<$pattern*>);
    if ($var_CompleteAddsuffix) {
	foreach (@files) {
	    if (-l $_) {
		$_ .= '@';
	    } elsif (-d _) {
		$_ .= '/';
	    } elsif (-x _) {
		$_ .= '*';
	    } elsif (-S _ || -p _) {
		$_ .= '=';
	    }
	}
    }
    return @files;
}

##
## For use by the user of the package. Called with a list of possible
## commands, will allow command completion on those commands, but only
## for the first word on a line.
## For example: &rl_basic_commands('set', 'quit', 'type', 'run');
##
## This is for people that want quick and simple command completion.
## A more thoughtful implementation would set $rl_completion_function
## to a routine that would look at the context of the word being completed
## and return the appropriate possibilities.
##
sub rl_basic_commands
{
     @rl_basic_commands = @_;
     $rl_completion_function = 'use_basic_commands';
}

##
## Print an array in columns like ls -C.  Originally based on stuff
## (lsC2.pl) by utashiro@sran230.sra.co.jp (Kazumasa Utashiro).
##
sub pretty_print_list
{
    my @list = @_;
    return unless @list;
    my ($lines, $columns, $mark, $index);

    ## find width of widest entry
    my $maxwidth = 0;
    grep(length > $maxwidth && ($maxwidth = length), @list);
    $maxwidth++;

    $columns = $maxwidth >= $rl_screen_width
	       ? 1 : int($rl_screen_width / $maxwidth);

    ## if there's enough margin to interspurse among the columns, do so.
    $maxwidth += int(($rl_screen_width % $maxwidth) / $columns);

    $lines = int((@list + $columns - 1) / $columns);
    $columns-- while ((($lines * $columns) - @list + 1) > $lines);

    $mark = $#list - $lines;
    for ($l = 0; $l < $lines; $l++) {
	for ($index = $l; $index <= $mark; $index += $lines) {
	    printf("%-$ {maxwidth}s", $list[$index]);
	}
   	print $term_OUT $list[$index] if $index <= $#list;
	print $term_OUT "\n\r";
    }
}

##----------------- Vi Routines --------------------------------

sub F_ViAcceptLine
{
    &F_AcceptLine();
    &F_ViInput();
}

# Repeat the most recent one of these vi commands:
#
#   a A c C d D i I p P r R s S x X ~ 
#
sub F_ViRepeatLastCommand {
    my($count) = @_;
    return &F_Ding if !$Last_vi_command;

    my @lastcmd = @$Last_vi_command;

    # Multiply @lastcmd's numeric arg by $count.
    unless ($count == 1) {

	my $n = '';
	while (@lastcmd and $lastcmd[0] =~ /^\d$/) {
	    $n *= 10;
	    $n += shift(@lastcmd);
	}
	$count *= $n unless $n eq '';
	unshift(@lastcmd, split(//, $count));
    }

    push(@Pending, @lastcmd);
}

sub F_ViMoveCursor
{
    my($count, $ord) = @_;

    my $new_cursor = &get_position($count, $ord, undef, $Vi_move_patterns);
    return &F_Ding if !defined $new_cursor;

    $D = $new_cursor;
}

sub F_ViFindMatchingParens {

    # Move to the first parens at or after $D
    my $old_d = $D;
    &forward_scan(1, q/[^[\](){}]*/);
    my $parens = substr($line, $D, 1);

    my $mate_direction = {
		    '('  =>  [ ')',  1 ],
		    '['  =>  [ ']',  1 ],
		    '{'  =>  [ '}',  1 ],
		    ')'  =>  [ '(', -1 ],
		    ']'  =>  [ '[', -1 ],
		    '}'  =>  [ '{', -1 ],

		}->{$parens};

    return &F_Ding() unless $mate_direction;

    my($mate, $direction) = @$mate_direction;

    my $lvl = 1;
    while ($lvl) {
	last if !$D && ($direction < 0);
	&F_ForwardChar($direction);
	last if &at_end_of_line;
	my $c = substr($line, $D, 1);
	if ($c eq $parens) {
	    $lvl++;
	}
	elsif ($c eq $mate) {
	    $lvl--;
	}
    }

    if ($lvl) {
	# We didn't find a match
	$D = $old_d;
	return &F_Ding();
    }
}

sub F_ViForwardFindChar {
    &do_findchar(1, 1, @_);
}

sub F_ViBackwardFindChar {
    &do_findchar(-1, 0, @_);
}

sub F_ViForwardToChar {
    &do_findchar(1, 0, @_);
}

sub F_ViBackwardToChar {
    &do_findchar(-1, 1, @_);
}

sub F_ViMoveCursorTo
{
    &do_findchar(1, -1, @_);
}

sub F_ViMoveCursorFind
{
    &do_findchar(1, 0, @_);
}


sub F_ViRepeatFindChar {
    my($n) = @_;
    return &F_Ding if !defined $Last_findchar;
    &findchar(@$Last_findchar, $n);
}

sub F_ViInverseRepeatFindChar {
    my($n) = @_;
    return &F_Ding if !defined $Last_findchar;
    my($c, $direction, $offset) = @$Last_findchar;
    &findchar($c, -$direction, $offset, $n);
}

sub do_findchar {
    my($direction, $offset, $n) = @_;
    my $c = &getc_with_pending;
    $c = &getc_with_pending if $c eq "\cV";
    return &F_ViCommandMode if $c eq "\e";
    $Last_findchar = [$c, $direction, $offset];
    &findchar($c, $direction, $offset, $n);
}

sub findchar {
    my($c, $direction, $offset, $n) = @_;
    my $old_d = $D;
    while ($n) {
	last if !$D && ($direction < 0);
	&F_ForwardChar($direction);
	last if &at_end_of_line;
	my $char = substr($line, $D, 1);
	$n-- if substr($line, $D, 1) eq $c;
    }
    if ($n) {
	# Not found
	$D = $old_d;
	return &F_Ding;
    }
    &F_ForwardChar($offset);
}

sub F_ViMoveToColumn {
    my($n) = @_;
    $D = 0;
    my $col = 1;
    while (!&at_end_of_line and $col < $n) {
	my $c = substr($line, $D, 1);
	if ($c eq "\t") {
	    $col += 7;
	    $col -= ($col % 8) - 1;
	}
	else {
	    $col++;
	}
	$D += &CharSize($D);
    }
}

sub start_dot_buf {
    my($count, $ord) = @_;
    $Dot_buf = [pack('c', $ord)];
    unshift(@$Dot_buf, split(//, $count)) if $count > 1;
    $Dot_state = &savestate;
}

sub end_dot_buf {
    # We've recognized an editing command

    # Save the command keystrokes for use by '.'
    $Last_vi_command = $Dot_buf;
    undef $Dot_buf;

    # Save the pre-command state for use by 'u' and 'U';
    $Vi_undo_state     = $Dot_state;
    $Vi_undo_all_state = $Dot_state if !$Vi_undo_all_state;

    # Make sure the current line is treated as new line for history purposes.
    $rl_HistoryIndex = $#rl_History + 1;
}

sub save_dot_buf {
    &start_dot_buf(@_);
    &end_dot_buf;
}

sub F_ViUndo {
    return &F_Ding unless defined $Vi_undo_state;
    my $state = &savestate;
    &getstate($Vi_undo_state);
    $Vi_undo_state = $state;
}

sub F_ViUndoAll {
    $Vi_undo_state = $Vi_undo_all_state;
    &F_ViUndo;
}

sub F_ViChange
{
    my($count, $ord) = @_;
    &start_dot_buf(@_);
    &do_delete($count, $ord, $Vi_change_patterns) || return();
    &vi_input_mode;
}

sub F_ViDelete
{
    my($count, $ord) = @_;
    &start_dot_buf(@_);
    &do_delete($count, $ord, $Vi_delete_patterns);
    &end_dot_buf;
}

sub do_delete {

    my($count, $ord, $poshash) = @_;

    my $other_end = &get_position($count, undef, $ord, $poshash);
    return &F_Ding if !defined $other_end;

    if ($other_end < 0) {
	# dd - delete entire line
	&kill_text(0, length($line), 1);
    }
    else {
	&kill_text($D, $other_end, 1);
    }

    1;    # True return value
}

sub F_ViDeleteChar {
    my($count) = @_;
    &save_dot_buf(@_);
    my $other_end = $D + $count;
    $other_end = length($line) if $other_end > length($line);
    &kill_text($D, $other_end, 1);
}

sub F_ViBackwardDeleteChar {
    my($count) = @_;
    &save_dot_buf(@_);
    my $other_end = $D - $count;
    $other_end = 0 if $other_end < 0;
    &kill_text($other_end, $D, 1);
    $D = $other_end;
}

##
## Prepend line with '#', add to history, and clear the input buffer
##     (this feature was borrowed from ksh).
##
sub F_ViSaveLine
{
    $line = '#'.$line;
    &redisplay();
    print $term_OUT "\r\n";
    &add_line_to_history;
    $line_for_revert = '';
    &get_line_from_history(scalar @rl_History);
    &F_ViInput();
}

#
# Come here if we see a non-positioning keystroke when a positioning
#     keystroke is expected.
#
sub F_ViNonPosition {
    # Not a positioning command - undefine the cursor to indicate the error
    #     to get_position().
    undef $D;
}

#
# Come here if we see <esc><char>, but *not* an arrow key or other
#     mapped sequence, when a positioning keystroke is expected.
#
sub F_ViPositionEsc {
    my($count, $ord) = @_;

    # We got <esc><char> in vipos mode.  Put <char> back onto the
    #     input stream and terminate the positioning command.
    unshift(@Pending, pack('c', $ord));
    &F_ViNonPosition;
}

# Interpret vi positioning commands
sub get_position {
    my ($count, $ord, $fullline_ord, $poshash) = @_;

    # Manipulate a copy of the cursor, not the real thing
    local $D = $D;

    # $ord (first character of positioning command) is an optional argument.
    $ord = ord(&getc_with_pending) if !defined $ord;

    # Detect double character (for full-line operation, e.g. dd)
    return -1 if defined $fullline_ord and $ord == $fullline_ord;

    my $re = $poshash->{$ord};

    if ($re) {
	my $c = pack('c', $ord);
	if (lc($c) eq 'b') {
	    &backward_scan($count, $re);
	}
	else {
	    &forward_scan($count, $re);
	}
    }
    else {
	# Move the local copy of the cursor
	&do_command($var_EditingMode{'vipos'}, $count, $ord);
    }

    # Return the new cursor (undef if illegal command)
    $D;
}

##
## Go to first non-space character of line.
##
sub F_ViFirstWord
{
    $D = 0;
    &forward_scan(1, q{\s+});
}

sub forward_scan {
    my($count, $re) = @_;
    while ($count--) {
	last unless substr($line, $D) =~ m{^($re)};
	$D += length($1);
    }
}

sub backward_scan {
    my($count, $re) = @_;
    while ($count--) {
	last unless substr($line, 0, $D) =~ m{($re)$};
	$D -= length($1);
    }
}

# Note: like the emacs case transforms, this doesn't work for
#       two-byte characters.
sub F_ViToggleCase {
    my($count) = @_;
    &save_dot_buf(@_);
    while ($count-- > 0) {
	substr($line, $D, 1) =~ tr/A-Za-z/a-zA-Z/;
	&F_ForwardChar(1);
	if (&at_end_of_line) {
	    &F_BackwardChar(1);
	    last;
	}
    }
}

sub F_ViPreviousHistory {
    &get_line_from_history($rl_HistoryIndex - 1);
}

sub F_ViNextHistory {
    &get_line_from_history($rl_HistoryIndex + 1);
}

# Go to the numbered history line, as listed by the 'H' command, i.e. the
#     current $line is line 1, the youngest line in @rl_History is 2, etc.
sub F_ViHistoryLine {
    my($n) = @_;
    &get_line_from_history(@rl_History - $n + 1);
}

sub get_line_from_history {
    my($n) = @_;
    return &F_Ding if $n < 0 or $n > @rl_History;
    return if $n == $rl_HistoryIndex;

    # If we're moving from the currently-edited line, save it for later.
    $line_for_revert = $line if $rl_HistoryIndex == @rl_History;

    # Get line from history buffer (or from saved edit line).
    $line = ($n == @rl_History) ? $line_for_revert : $rl_History[$n];
    $D = 0;

    # Subsequent 'U' will bring us back to this point.
    $Vi_undo_all_state = &savestate;

    $rl_HistoryIndex = $n;
}

sub F_ViPrintHistory {
    my($count) = @_;

    $count = 20 if $count == 1;             # Default - assume 'H', not '1H'
    my $end = $rl_HistoryIndex + $count/2;
    $end = @rl_History if $end > @rl_History;
    my $start = $end - $count + 1;
    $start = 0 if $start < 0;

    my $lmh = length $rl_MaxHistorySize;

    my $lspace = ' ' x ($lmh+3);
    my $hdr = "$lspace----- (Use '<num>G' to retrieve command <num>) -----\n";

    print "\n", $hdr;
    print $lspace, ". . .\n" if $start > 0;
    my $i;
    for $i ($start .. $end) {
	print + ($i == $rl_HistoryIndex) ? '>' : ' ',

		sprintf("%${lmh}d: ", @rl_History - $i + 1),

		($i < @rl_History)       ? $rl_History[$i] :
		($i == $rl_HistoryIndex) ? $line           :
		                           $line_for_revert,

		"\n";
    }
    print $lspace, ". . .\n" if $end < @rl_History;
    print $hdr;

    &force_redisplay();

    &F_ViInput() if $line eq '';
}

# Redisplay the line, without attempting any optimization
sub force_redisplay {
    local $force_redraw = 1;
    &redisplay(@_);
}

# Search history for matching string.  As with vi in nomagic mode, the
#     ^, $, \<, and \> positional assertions, the \* quantifier, the \.
#     character class, and the \[ character class delimiter all have special
#     meaning here.
sub F_ViSearch {
    my($n, $ord) = @_;

    my $c = pack('c', $ord);

    my $str = &get_vi_search_str($c);
    if (!defined $str) {
	# Search aborted by deleting the '/' at the beginning of the line
	return &F_ViInput() if $line eq '';
	return();
    }

    # Null string repeats last search
    if ($str eq '') {
	return &F_Ding unless defined $Vi_search_re;
    }
    else {
	# Convert to a regular expression.  Interpret $str Like vi in nomagic
	#     mode: '^', '$', '\<', and '\>' positional assertions, '\*' 
	#     quantifier, '\.' and '\[]' character classes.

	my @chars = ($str =~ m{(\\?.)}g);
	my(@re, @tail);
	unshift(@re,   shift(@chars)) if @chars and $chars[0]  eq '^';
	push   (@tail, pop(@chars))   if @chars and $chars[-1] eq '$';
	my $in_chclass;
	my %chmap = (
	    '\<' => '\b(?=\w)',
	    '\>' => '(?<=\w)\b',
	    '\*' => '*',
	    '\[' => '[',
	    '\.' => '.',
	);
	my $ch;
	foreach $ch (@chars) {
	    if ($in_chclass) {
		# Any backslashes in vi char classes are literal
		push(@re, "\\") if length($ch) > 1;
		push(@re, $ch);
		$in_chclass = 0 if $ch =~ /\]$/;
	    }
	    else {
		push(@re, (length $ch == 2) ? ($chmap{$ch} || $ch) :
			  ($ch =~ /^\w$/)   ? $ch                  :
			                      ("\\", $ch));
		$in_chclass = 1 if $ch eq '\[';
	    }
	}
	my $re = join('', @re, @tail);
	$Vi_search_re = q{$re};
    }

    local $reverse = $Vi_search_reverse = ($c eq '/') ? 1 : 0;
    &do_vi_search();
}

sub F_ViRepeatSearch {
    my($n, $ord) = @_;
    my $c = pack('c', $ord);
    return &F_Ding unless defined $Vi_search_re;
    local $reverse = $Vi_search_reverse;
    $reverse ^= 1 if $c eq 'N';
    &do_vi_search();
}

## returns a new $i or -1 if not found.
sub vi_search { 
    my ($i) = @_;
    return -1 if $i < 0 || $i > $#rl_History; 	 ## for safety
    while (1) {
	return $i if $rl_History[$i] =~ /$Vi_search_re/;
	if ($reverse) {
	    return -1 if $i-- == 0;
	} else {
	    return -1 if $i++ == $#rl_History;
	}
    }
}

sub do_vi_search {
    my $incr = $reverse ? -1 : 1;

    my $i = &vi_search($rl_HistoryIndex + $incr);
    return &F_Ding if $i < 0;                  # Not found.

    $rl_HistoryIndex = $i;
    ($D, $line) = (0, $rl_History[$rl_HistoryIndex]);
}

# Using local $line, $D, and $prompt, get and return the string to search for.
sub get_vi_search_str {
    my($c) = @_;

    local $prompt = $prompt . $c;
    local ($line, $D) = ('', 0);
    &redisplay();

    # Gather a search string in our local $line.
    while ($lastcommand ne 'F_ViEndSearch') {
	&do_command($var_EditingMode{'visearch'}, 1, ord(&getc_with_pending));
	&redisplay();

	# We've backspaced past beginning of line
	return undef if !defined $line;
    }
    $line;
}

sub F_ViEndSearch {}

sub F_ViSearchBackwardDeleteChar {
    if ($line eq '') {
	# Backspaced past beginning of line - terminate search mode
	undef $line;
    }
    else {
	&F_BackwardDeleteChar(@_);
    }
}

##
## Kill entire line and enter input mode
##
sub F_ViChangeEntireLine
{
    &start_dot_buf(@_);
    kill_text(0, length($line), 1);
    &vi_input_mode;
}

##
## Kill characters and enter input mode
##
sub F_ViChangeChar
{
    &start_dot_buf(@_);
    &F_DeleteChar(@_);
    &vi_input_mode;
}

sub F_ViReplaceChar
{
    &start_dot_buf(@_);
    my $c = &getc_with_pending;
    $c = &getc_with_pending if $c eq "\cV";   # ctrl-V
    return &F_ViCommandMode if $c eq "\e";
    &end_dot_buf;

    local $InsertMode = 0;
    local $D = $D;                  # Preserve cursor position
    &F_SelfInsert(1, ord($c));
}

##
## Kill from cursor to end of line and enter input mode
##
sub F_ViChangeLine
{
    &start_dot_buf(@_);
    &F_KillLine(@_);
    &vi_input_mode;
}

sub F_ViDeleteLine
{
    &save_dot_buf(@_);
    &F_KillLine(@_);
}

sub F_ViPut
{
    my($count) = @_;
    &save_dot_buf(@_);
    my $text2add = $KillBuffer x $count;
    my $ll = length($line);
    $D++;
    $D = $ll if $D > $ll;
    substr($line, $D, 0) = $KillBuffer x $count;
    $D += length($text2add) - 1;
}

sub F_ViPutBefore
{
    &save_dot_buf(@_);
    &TextInsert($_[0], $KillBuffer);
}

sub F_ViYank
{
    my($count, $ord) = @_;
    my $pos = &get_position($count, undef, $ord, $Vi_yank_patterns);
    &F_Ding if !defined $pos;
    if ($pos < 0) {
	# yy
	&F_ViYankLine;
    }
    else {
	my($from, $to) = ($pos > $D) ? ($D, $pos) : ($pos, $D);
	$KillBuffer = substr($line, $from, $to-$from);
    }
}

sub F_ViYankLine
{
    $KillBuffer = $line;
}

sub F_ViInput
{
    @_ = (1, ord('i')) if !@_;
    &start_dot_buf(@_);
    &vi_input_mode;
}

sub F_ViBeginInput
{
    &start_dot_buf(@_);
    &F_BeginningOfLine;
    &vi_input_mode;
}

sub F_ViReplaceMode
{
    &start_dot_buf(@_);
    $InsertMode = 0;
    $var_EditingMode = $var_EditingMode{'vi'};
    $Vi_mode = 1;
}

sub vi_input_mode
{
    $InsertMode = 1;
    $var_EditingMode = $var_EditingMode{'vi'};
    $Vi_mode = 1;
}

# The previous keystroke was an escape, but the sequence was not recognized
#     as a mapped sequence (like an arrow key).  Enter vi comand mode and
#     process this keystroke.
sub F_ViAfterEsc {
    my($n, $ord) = @_;
    &F_ViCommandMode;
    &do_command($var_EditingMode, 1, $ord);
}

sub F_ViAppend
{
    &start_dot_buf(@_);
    &vi_input_mode;
    &F_ForwardChar;
}

sub F_ViAppendLine
{
    &start_dot_buf(@_);
    &vi_input_mode;
    &F_EndOfLine;
}

sub F_ViCommandMode
{
    $var_EditingMode = $var_EditingMode{'vicmd'};
    $Vi_mode = 1;
}

sub F_ViAcceptInsert {
    &F_ViEndInsert;
    &F_ViAcceptLine;
}

sub F_ViEndInsert
{
    if ($Dot_buf) {
	if ($line eq '' and $Dot_buf->[0] eq 'i') {
	    # We inserted nothing into an empty $line - assume it was a
	    #     &F_ViInput() call with no arguments, and don't save command.
	    undef $Dot_buf;
	}
	else {
	    # Regardless of which keystroke actually terminated this insert
	    #     command, replace it with an <esc> in the dot buffer.
	    @{$Dot_buf}[-1] = "\e";
	    &end_dot_buf;
	}
    }
    &F_ViCommandMode;
    &F_BackwardChar;
}

sub F_ViDigit {
    my($count, $ord) = @_;

    my $n = 0;
    my $ord0 = ord('0');
    while (1) {

	$n *= 10;
	$n += $ord - $ord0;

	my $c = &getc_with_pending;
	return unless defined $c;
	$ord = ord($c);
	last unless $c =~ /^\d$/;
    }

    $n *= $count;                   # So  2d3w  deletes six words
    $n = $rl_max_numeric_arg if $n > $rl_max_numeric_arg;

    &do_command($var_EditingMode, $n, $ord);
}

sub F_ViComplete {
    my($n, $ord) = @_;

    $Dot_state = &savestate;     # Completion is undo-able
    undef $Dot_buf;              #       but not redo-able

    my $ch;
    while (1) {

	&F_Complete() or return;

	# Vi likes the cursor one character right of where emacs like it.
	&F_ForwardChar(1);
	&force_redisplay();

	# Look ahead to the next input keystroke.
	$ch = &getc_with_pending();
	last unless ord($ch) == $ord;   # Not a '\' - quit.

	# Another '\' was typed - put the cursor back where &F_Complete left
	#     it, and try again.
	&F_BackwardChar(1);
	$lastcommand = 'F_Complete';   # Play along with &F_Complete's kludge
    }
    unshift(@Pending, $ch);      # Unget the lookahead keystroke

    # Successful completion - enter input mode with cursor beyond end of word.
    &vi_input_mode;
}

sub F_ViInsertPossibleCompletions {
    $Dot_state = &savestate;     # Completion is undo-able
    undef $Dot_buf;              #       but not redo-able

    &complete_internal('*') or return;

    # Successful completion - enter input mode with cursor beyond end of word.
    &F_ForwardChar(1);
    &vi_input_mode;
}

sub F_ViPossibleCompletions {

    # List possible completions
    &complete_internal('?');

    # Enter input mode with cursor where we left off.
    &F_ForwardChar(1);
    &vi_input_mode;
}

1;
__END__
