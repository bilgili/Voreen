find .. -name *.cpp -exec svn propset svn:eol-style native {} \;
find .. -name *.h -exec svn propset svn:eol-style native {} \;
find .. -name *.c -exec svn propset svn:eol-style native {} \;
find .. -name *.frag -exec svn propset svn:eol-style native {} \;
find .. -name *.vert -exec svn propset svn:eol-style native {} \;
find .. -name *.cl -exec svn propset svn:eol-style native {} \;
find .. -name *.pro -exec svn propset svn:eol-style native {} \;
find .. -name *.txt -exec svn propset svn:eol-style native {} \;
find .. -name *.py -exec svn propset svn:eol-style native {} \;

