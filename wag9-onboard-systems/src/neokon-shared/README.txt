This directory is the top-level directory in the neokon-shared repository,
meant to be used by means of "git subrepo" from other projects.

As its name suggests, this repository holds the code that's shared between
several Neokon-Baltija projects. There are no build scripts, because different
source files have different dependencies, and the projects using them also
have different dependencies. Satisfying the dependencies of each particular
source file from this repository is up to that project.

For git subrepo documentation, see the project's home page:
https://github.com/ingydotnet/git-subrepo