// object "outside" .hxx template

#include "IDB/IDB.h"
#include "IO/Object.h"

#ifdef STATIC
#undef STATIC
#endif
#define STATIC static

// class definitions needed by this class
<% foreach my $navigator (@navigators) { %>
  class <%=$navigator->{object}%>;
<% } %>

// child objects
<% foreach my $child (@children) { %>
  class <%=$child->{object}%>;
<% } %>

