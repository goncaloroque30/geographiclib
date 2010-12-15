function height = geoidheight(latlong, geoidname, geoiddir)
%geoidheight  Compute geoid height
%
%   CAUTION: THIS MAY CAUSE MATLAB TO CRASH!!  This occurs with Matlab
%   2008a when using Visual Studio 2008 as the compiler.
%
%   height = geoidheight(latlong);
%   height = geoidheight(latlong, geoidname);
%   height = geoidheight(latlong, geoidname, geoiddir);
%
%   latlong is an M x 2 matrix
%       latitude = latlong(:,1) in degrees
%       longitude = latlong(:,2) in degrees
%   geoidname is the name of the geoid; choices are (default egm96-5)
%       egm84-30  egm84-15
%       egm96-15  egm96-5
%       egm2008-5 egm2008-2_5 egm2008-1
%   geoiddir is the direcortory containing the geoid models (default empty
%       string meaning system default)
%
%   height is an M x 3 matrix
%       geoidheight = utmups(:,1) in meters
%       gradn = utmups(:,2) gradient of height in northerly direction
%       grade = utmups(:,3) gradient of height in easterly direction
%
%   This is an interface to the GeographicLib C++ routine
%       Geoid::operator()
%   See the documentation on this function for more information.
  error('Error: executing .m file instead of compiled routine');
end
% geoidheight.m
% Matlab .m file for looking up geoid heights
%
% Copyright (c) Charles Karney (2010) <charles@karney.com> and licensed under
% the LGPL.  For more information, see http://geographiclib.sourceforge.net/
%
% $Id$