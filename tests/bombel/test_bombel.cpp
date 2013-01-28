/** 
 * @file
 * @copyright University of Warsaw
 * @section LICENSE
 * GPLv3+ (see the COPYING file or http://www.gnu.org/licenses/)
 *
 * @brief a bombel 
 */

// advection (<> should be used instead of "" in normal usage) 
#include "advoocat/mpdata_2d.hpp"
#include "advoocat/solver_pressure.hpp"
#include "advoocat/cyclic_2d.hpp"
//gradient
#include "advoocat/nabla_formulae.hpp"
//physical constants
#include "advoocat/phc.hpp"
//theta->pressure
#include "advoocat/diagnose_formulae.hpp"

// plotting
#define GNUPLOT_ENABLE_BLITZ
#include <gnuplot-iostream/gnuplot-iostream.h>

// auto-deallocating containers
#include <boost/ptr_container/ptr_map.hpp>
#include <boost/assign/ptr_map_inserter.hpp>

enum {u, w, tht};  //eqations
enum {x, z};       //dimensions

template <int n_iters, typename real_t>
using parent = 
pressure_solver<
  inhomo_solver<
    solvers::mpdata_2d<
      n_iters, 
      cyclic_2d<x, real_t>, 
      cyclic_2d<z, real_t>,
      3, 
      real_t
    >
  >, u, w, tht, x, z
>;

template <int n_iters, typename real_t = float>
class bombel : public parent<n_iters, real_t>
{

  real_t Tht_amb;

  void forcings(real_t dt)  //explicit forcings (to be applied before the eliptic solver)
  {
    auto W   = this->state(w);
    auto Tht = this->state(tht);

    W += (dt * si:: seconds) * phc::g<real_t>() * si::seconds / si::metres * (Tht - Tht_amb) / Tht_amb;
  }

  public:

  bombel(int nx, int ny, real_t dt, real_t Tht_amb, real_t Prs_amb) :
    parent<n_iters, real_t>(nx, ny, dt, Prs_amb),
    Tht_amb(Tht_amb)
  {}
};

int main() 
{
  const int nx = 10, ny = 10, nt = 2, n_out=1;
//  const int nx = 50, ny = 50, nt = 41, n_out=10;
  using real_t = float;
  const real_t dt = .1;

  rng_t i(0, nx-1);
  rng_t j(0, ny-1);
  const real_t halo = 1;  

  //ambient state (constant thoughout the domain)
  real_t Tht_amb = 300;  
  real_t Prs_amb = diagnose::p(Tht_amb);

  bombel<2> solver(nx, ny, dt, Tht_amb, Prs_amb);

  // initial condition
  {
    blitz::firstIndex i;
    blitz::secondIndex j;

    solver.state(tht) = Tht_amb 
      + exp( -sqr(i-nx/2.) / (2.*pow(nx/10, 2))
             -sqr(j-ny/3.) / (2.*pow(ny/10, 2)) )
    ;
    solver.state(u) = real_t(0); 
    solver.state(w) = real_t(0); 
  }

  //ploting
  Gnuplot gp;
  gp << "reset\n"
     << "set term svg size 2000,1000 dynamic\n"
     << "set output 'figure.svg'\n"
     << "set multiplot layout 3,5 columnsfirst\n"
     << "set grid\n"
     << "set xlabel 'X'\n"
     << "set ylabel 'Y'\n"
     << "set xrange [0:" << nx-1 << "]\n"
     << "set yrange [0:" << ny-1 << "]\n"
     // progressive-rock connoisseur palette ;)
     << "set palette defined (0 '#ffffff', 1 '#993399', 2 '#00CCFF', 3 '#66CC00', 4 '#FFFF00', 5 '#FC8727', 6 '#FD0000')\n" 
     << "set view map\n"
     << "set key font \",5\"\n "
     << "set contour base\n" 
     << "set nosurface\n"
     << "set cntrparam levels 0\n";

  std::string binfmt;
  binfmt = gp.binfmt(solver.state());

  // integration
//  for (int t = 1; t <= 5; ++t)
//  {
int t=2;
    solver.solve(t); // 1 tymczasowo
//   if (t % n_out == 0 /*&& t != 0*/)  
//   {    
//      gp << "set title 'tht @ t=" << t+1 << "'\n"
      gp << "set title 'tht @ t=" << int(t * dt) << "'\n"
//         << "set cbrange [300:301]\n"
         << "splot '-' binary" << binfmt << "with image notitle\n";
      gp.sendBinary(solver.state(tht).copy());
//      gp << "set title 'w @ t=" << t+1 << "'\n"
      gp << "set title 'u @ t=" << int(t * dt) << "'\n"
//         << "set cbrange [0:.01]\n"
         << "splot '-' binary" << binfmt << "with image notitle\n";
      gp.sendBinary(solver.state(u).copy());
      gp << "set title 'w @ t=" << int(t * dt) << "'\n"
//         << "set cbrange [0:.01]\n"
         << "splot '-' binary" << binfmt << "with image notitle\n";
      gp.sendBinary(solver.state(w).copy());
//   }
//  }
};