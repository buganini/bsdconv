use 5.008009;
use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    NAME              => 'bsdconv',
    VERSION_FROM      => 'lib/bsdconv.pm', # finds $VERSION
    PREREQ_PM         => {},
    ($] >= 5.005 ?     ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM  => 'lib/bsdconv.pm', # retrieve abstract from module
       AUTHOR         => 'Buganini <buganini@buganini@gmail.com>') : ()),
    LIBS              => ['-L/usr/local/lib -lbsdconv'],
    DEFINE            => '', # e.g., '-DHAVE_SOMETHING'
    INC               => '-I. -I/usr/local/include',
	# Un-comment this if you add C files to link with later:
    # OBJECT            => '$(O_FILES)', # link all the C files too
);
