/**
 * \file DMS.cpp
 * \brief Implementation for GeographicLib::DMS class
 *
 * Copyright (c) Charles Karney (2008, 2009, 2010, 2011) <charles@karney.com>
 * and licensed under the MIT/X11 License.  For more information, see
 * http://geographiclib.sourceforge.net/
 **********************************************************************/

#include <GeographicLib/DMS.hpp>
#include <algorithm>

#define GEOGRAPHICLIB_DMS_CPP "$Id: 96750965935bfd5404dfa361e7c4b3a9ee207135 $"

RCSID_DECL(GEOGRAPHICLIB_DMS_CPP)
RCSID_DECL(GEOGRAPHICLIB_DMS_HPP)

namespace GeographicLib {

  using namespace std;

  const string DMS::hemispheres_ = "SNWE";
  const string DMS::signs_ = "-+";
  const string DMS::digits_ = "0123456789";
  const string DMS::dmsindicators_ = "D'\":";
  const string DMS::components_[] = {"degrees", "minutes", "seconds"};

  Math::real DMS::Decode(const std::string& dms, flag& ind) {
    string errormsg;
    string dmsa = dms;
    replace(dmsa, "\xc2\xb0", 'd'); // degree symbol (U+00b0 = UTF-8 c2 b0)
    replace(dmsa, "\xc2\xba", 'd'); // alt symbol (U+00ba = UTF-8 c2 ba)
    replace(dmsa, "\xe2\x80\xb2", '\''); // prime (U+2032 = UTF-8 e2 80 b2)
    replace(dmsa, "\xe2\x80\xb3", '"');  // dbl prime (U+2033 = UTF-8 e2 80 b3)
    replace(dmsa, "\xb0", 'd');          // bare degree symbol (b0)
    replace(dmsa, "\xba", 'd');          // bare alt symbol (ba)
    replace(dmsa, "''", '"');            // '' -> "
    do {                       // Executed once (provides the ability to break)
      int sign = 1;
      unsigned
        beg = 0,
        end = unsigned(dmsa.size());
      while (beg < end && isspace(dmsa[beg]))
        ++beg;
      while (beg < end && isspace(dmsa[end - 1]))
        --end;
      flag ind1 = NONE;
      int k = -1;
      if (end > beg && (k = lookup(hemispheres_, dmsa[beg])) >= 0) {
        ind1 = (k / 2) ? LONGITUDE : LATITUDE;
        sign = k % 2 ? 1 : -1;
        ++beg;
      }
      if (end > beg && (k = lookup(hemispheres_, dmsa[end-1])) >= 0) {
        if (k >= 0) {
          if (ind1 != NONE) {
            if (toupper(dmsa[beg - 1]) == toupper(dmsa[end - 1]))
              errormsg = "Repeated hemisphere indicators " + str(dmsa[beg - 1])
                + " in " + dmsa.substr(beg - 1, end - beg + 1);
            else
              errormsg = "Contradictory hemisphere indicators "
                + str(dmsa[beg - 1]) + " and " + str(dmsa[end - 1]) + " in "
                + dmsa.substr(beg - 1, end - beg + 1);
            break;
          }
          ind1 = (k / 2) ? LONGITUDE : LATITUDE;
          sign = k % 2 ? 1 : -1;
          --end;
        }
      }
      if (end > beg && (k = lookup(signs_, dmsa[beg])) >= 0) {
        if (k >= 0) {
          sign *= k ? 1 : -1;
          ++beg;
        }
      }
      if (end == beg) {
        errormsg = "Empty or incomplete DMS string " + dmsa;
        break;
      }
      real ipieces[] = {0, 0, 0};
      real fpieces[] = {0, 0, 0};
      unsigned npiece = 0;
      real icurrent = 0;
      real fcurrent = 0;
      unsigned ncurrent = 0, p = beg;
      bool pointseen = false;
      unsigned digcount = 0;
      while (p < end) {
        char x = dmsa[p++];
        if ((k = lookup(digits_, x)) >= 0) {
          ++ncurrent;
          if (digcount > 0)
            ++digcount;         // Count of decimal digits
          else
            icurrent = 10 * icurrent + k;
        } else if (x == '.') {
          if (pointseen) {
            errormsg = "Multiple decimal points in "
              + dmsa.substr(beg, end - beg);
            break;
          }
          pointseen = true;
          digcount = 1;
        } else if ((k = lookup(dmsindicators_, x)) >= 0) {
          if (k >= 3) {
            if (p == end) {
              errormsg = "Illegal for : to appear at the end of " +
                dmsa.substr(beg, end - beg);
              break;
            }
            k = npiece;
          }
          if (unsigned(k) == npiece - 1) {
            errormsg = "Repeated " + components_[k] +
              " component in " + dmsa.substr(beg, end - beg);
            break;
          } else if (unsigned(k) < npiece) {
            errormsg = components_[k] + " component follows "
              + components_[npiece - 1] + " component in "
              + dmsa.substr(beg, end - beg);
            break;
          }
          if (ncurrent == 0) {
            errormsg = "Missing numbers in " + components_[k] +
              " component of " + dmsa.substr(beg, end - beg);
            break;
          }
          if (digcount > 1) {
            istringstream s(dmsa.substr(p - digcount - 1, digcount));
            s >> fcurrent;
          }
          ipieces[k] = icurrent;
          fpieces[k] = icurrent + fcurrent;
          if (p < end) {
            npiece = k + 1;
            icurrent = fcurrent = 0;
            ncurrent = digcount = 0;
          }
        } else if (lookup(signs_, x) >= 0) {
          errormsg = "Internal sign in DMS string "
            + dmsa.substr(beg, end - beg);
          break;
        } else {
          errormsg = "Illegal character " + str(x) + " in DMS string "
            + dmsa.substr(beg, end - beg);
          break;
        }
      }
      if (!errormsg.empty())
        break;
      if (lookup(dmsindicators_, dmsa[p - 1]) < 0) {
        if (npiece >= 3) {
          errormsg = "Extra text following seconds in DMS string "
            + dmsa.substr(beg, end - beg);
          break;
        }
        if (ncurrent == 0) {
          errormsg = "Missing numbers in trailing component of "
            + dmsa.substr(beg, end - beg);
          break;
        }
        if (digcount > 1) {
          istringstream s(dmsa.substr(p - digcount, digcount));
          s >> fcurrent;
        }
        ipieces[npiece] = icurrent;
        fpieces[npiece] = icurrent + fcurrent;
      }
      if (pointseen && digcount == 0) {
        errormsg = "Decimal point in non-terminal component of "
          + dmsa.substr(beg, end - beg);
        break;
      }
      // Note that we accept 59.999999... even though it rounds to 60.
      if (ipieces[1] >= 60) {
        errormsg = "Minutes " + str(fpieces[1]) + " not in range [0, 60)";
        break;
      }
      if (ipieces[2] >= 60) {
        errormsg = "Seconds " + str(fpieces[2]) + " not in range [0, 60)";
        break;
      }
      ind = ind1;
      // Assume check on range of result is made by calling routine (which
      // might be able to offer a better diagnostic).
      return real(sign) * (fpieces[0] + (fpieces[1] + fpieces[2] / 60) / 60);
    } while (false);
    real val = NumMatch(dmsa);
    if (val == 0)
      throw GeographicErr(errormsg);
    else
      ind = NONE;
    return val;
  }

  Math::real DMS::NumMatch(const std::string& s) {
    if (s.length() < 3)
      return 0;
    string t;
    t.resize(s.length());
    for (size_t i = s.length(); i--;)
      t[i] = toupper(s[i]);
    int sign = t[0] == '-' ? -1 : 1;
    string::size_type p0 = t[0] == '-' || t[0] == '+' ? 1 : 0;
    string::size_type p1 = t.find_last_not_of('0');
    if (p1 == string::npos || p1 + 1 < p0 + 3)
      return 0;
    // Strip off sign and trailing 0s
    t = t.substr(p0, p1 + 1 - p0);  // Length at least 3
    if (t == "NAN" || t == "1.#QNAN" || t == "1.#SNAN" || t == "1.#IND" ||
        t == "1.#R")
      return sign * Math::NaN<real>();
    else if (t == "INF" || t == "1.#INF")
      return sign * Math::infinity<real>();
    return 0;
  }

  Math::real DMS::Decode(const std::string& str) {
    istringstream is(str);
    string errormsg;
    real num;
    do {                       // Executed once (provides the ability to break)
      if (!(is >> num)) {
        errormsg = "Could not read number: " + str;
        break;
      }
      // On some platforms, is >> num gobbles final E in 1234E, so look for
      // last character which is legal as the final character in a number
      // (digit or period).
      int pos = int(is.tellg()); // Returns -1 at end of string?
      if (pos < 0)
        pos = int(str.size());
      pos = min(pos, int(str.find_last_of("0123456789.")) + 1);
      if (pos != int(str.size())) {
        errormsg = "Extra text " + str.substr(pos) + " in number " + str;
        break;
      }
      return num;
    } while (false);
    num = NumMatch(str);
    if (num == 0)
      throw GeographicErr(errormsg);
    return num;
  }

  Math::real DMS::DecodeFraction(const std::string& str) {
    string::size_type delim = str.find('/');
    if (!(delim != string::npos && delim >= 1 && delim + 2 <= str.size()))
      return Decode(str);
    else
      // delim in [1, size() - 2]
      return Decode(str.substr(0, delim)) / Decode(str.substr(delim + 1));
  }

  void DMS::DecodeLatLon(const std::string& stra, const std::string& strb,
                         real& lat, real& lon, bool swaplatlong) {
    real a, b;
    flag ia, ib;
    a = Decode(stra, ia);
    b = Decode(strb, ib);
    if (ia == NONE && ib == NONE) {
      // Default to lat, long unless swaplatlong
      ia = swaplatlong ? LONGITUDE : LATITUDE;
      ib = swaplatlong ? LATITUDE : LONGITUDE;
    } else if (ia == NONE)
      ia = flag(LATITUDE + LONGITUDE - ib);
    else if (ib == NONE)
      ib = flag(LATITUDE + LONGITUDE - ia);
    if (ia == ib)
      throw GeographicErr("Both " + stra + " and "
                          + strb + " interpreted as "
                          + (ia == LATITUDE ? "latitudes" : "longitudes"));
    real
      lat1 = ia == LATITUDE ? a : b,
      lon1 = ia == LATITUDE ? b : a;
    if (lat1 < -90 || lat1 > 90)
      throw GeographicErr("Latitude " + str(lat1) + "d not in [-90d, 90d]");
    if (lon1 < -180 || lon1 > 360)
      throw GeographicErr("Latitude " + str(lon1)
                          + "d not in [-180d, 360d]");
    if (lon1 >= 180)
      lon1 -= 360;
    lat = lat1;
    lon = lon1;
  }

  Math::real DMS::DecodeAngle(const std::string& angstr) {
    flag ind;
    real ang = Decode(angstr, ind);
    if (ind != NONE)
      throw GeographicErr("Arc angle " + angstr
                          + " includes a hemisphere, N/E/W/S");
    return ang;
  }

  Math::real DMS::DecodeAzimuth(const std::string& azistr) {
    flag ind;
    real azi = Decode(azistr, ind);
    if (ind == LATITUDE)
      throw GeographicErr("Azimuth " + azistr
                          + " has a latitude hemisphere, N/S");
    if (azi < -180 || azi > 360)
      throw GeographicErr("Azimuth " + azistr + " not in range [-180d, 360d]");
    if (azi >= 180) azi -= 360;
    return azi;
  }

  string DMS::Encode(real angle, component trailing, unsigned prec, flag ind) {
    // Assume check on range of input angle has been made by calling
    // routine (which might be able to offer a better diagnostic).
    if (!Math::isfinite(angle))
      return angle < 0 ? string("-inf") :
        (angle > 0 ? string("inf") : string("nan"));

    // 15 - 2 * trailing = ceiling(log10(2^53/90/60^trailing)).
    // This suffices to give full real precision for numbers in [-90,90]
    prec = min(15 - 2 * unsigned(trailing), prec);
    real scale = 1;
    for (unsigned i = 0; i < unsigned(trailing); ++i)
      scale *= 60;
    for (unsigned i = 0; i < prec; ++i)
      scale *= 10;
    if (ind == AZIMUTH)
      angle -= floor(angle/360) * 360;
    int sign = angle < 0 ? -1 : 1;
    angle *= sign;

    // Break off integer part to preserve precision in manipulation of
    // fractional part.
    real
      idegree = floor(angle),
      fdegree = floor((angle - idegree) * scale + real(0.5)) / scale;
    if (fdegree >= 1) {
      idegree += 1;
      fdegree -= 1;
    }
    real pieces[3] = {fdegree, 0, 0};
    for (unsigned i = 1; i <= unsigned(trailing); ++i) {
      real
        ip = floor(pieces[i - 1]),
        fp = pieces[i - 1] - ip;
      pieces[i] = fp * 60;
      pieces[i - 1] = ip;
    }
    pieces[0] += idegree;
    ostringstream s;
    s << fixed  << setfill('0');
    if (ind == NONE && sign < 0)
      s << '-';
    switch (trailing) {
    case DEGREE:
      if (ind != NONE)
        s << setw(1 + min(int(ind), 2) + prec + (prec ? 1 : 0));
      s << setprecision(prec) << pieces[0];
      // Don't include degree designator (d) if it is the trailing component.
      break;
    default:
      if (ind != NONE)
        s << setw(1 + min(int(ind), 2));
      s << setprecision(0) << pieces[0] << char(tolower(dmsindicators_[0]));
      switch (trailing) {
      case MINUTE:
        s << setw(2 + prec + (prec ? 1 : 0)) << setprecision(prec)
          << pieces[1] <<  char(tolower(dmsindicators_[1]));
        break;
      case SECOND:
        s << setw(2) << pieces[1] <<  char(tolower(dmsindicators_[1]))
          << setw(2 + prec + (prec ? 1 : 0)) << setprecision(prec)
          << pieces[2] <<  char(tolower(dmsindicators_[2]));
        break;
      default:
        break;
      }
    }
    if (ind != NONE && ind != AZIMUTH)
      s << hemispheres_[(ind == LATITUDE ? 0 : 2) + (sign < 0 ? 0 : 1)];
    return s.str();
  }

} // namespace GeographicLib
