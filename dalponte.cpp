/* v2.5 */

#include <Rcpp.h>
#include <algorithm>
#include "lidR/Point.h"
using namespace Rcpp;
using namespace lidR;

typedef Point3D<int, int, int, int> Pixeli;
typedef Point3D<int, int, double, int> Pixeld;

//[[Rcpp::export]]
IntegerMatrix C_dalponte2016(NumericMatrix Image, IntegerMatrix Seeds, double th_seed, double th_crown, double th_tree, double DIST)
{
  bool grown = true;
  bool expend;
  int nrow  = Image.nrow();
  int ncol  = Image.ncol();

  std::vector<Pixeld> neighbours(4);

  if (Seeds.nrow() != nrow || Seeds.ncol() != ncol)
    throw std::runtime_error(std::string("Error: unexpected internal error: different matrix sizes.")); // # nocov


  IntegerMatrix Region     = clone(Seeds);
  IntegerMatrix Regiontemp = clone(Seeds);

  std::map<int, Pixeli> seeds;                                     // Stores all the seed as Pixel object
  std::map<int, double> sum_height;                                // Stores the sum of the elevation of each pixel of a tree (to compute mean height)
  std::map<int, int> npixel;

  for (int i = 0 ; i < nrow ; i++)
  {
    for (int j = 0 ; j < ncol ; j++)
    {
      if (Seeds(i,j) != 0)
      {
        seeds[Seeds(i,j)] = Pixeli(i,j, Seeds(i,j));
        sum_height[Seeds(i,j)] = Image(i,j);
        npixel[Seeds(i,j)] = 1;
      }
    }
  }

  while (grown)
  {
    grown = false;

    for (int r = 1 ; r < nrow-1 ; r++)                                 // Loops across the entiere image
    {
      for(int k = 1 ; k < ncol-1 ; k++)
      {
        if(Region(r, k) != 0)                                          // If the pixel is already labeled
        {
          int id = Region(r, k);                                       // id of the crown for the current pixel

          Pixeli seed  = seeds[id];                                 // Get the seed with the label id
          double hSeed    = Image(seed.x, seed.y);                     // Seed height
          double mhCrown  = sum_height[id]/npixel[id];                 // Mean height of the crown

          // Elevation of the 4 neighbours
          neighbours[0] = Pixeld(r-1, k, Image(r-1, k));
          neighbours[1] = Pixeld(r, k-1, Image(r, k-1));
          neighbours[2] = Pixeld(r, k+1, Image(r, k+1));
          neighbours[3] = Pixeld(r+1, k, Image(r+1, k));

          for(unsigned int i = 0 ; i < neighbours.size() ; i++)                  // For each neighboring pixel
          {
            Pixeld px = neighbours[i];

            if (px.z > th_tree)                                       // The pixel is higher than the minimum value)
            {
              expend =
                px.z > hSeed*th_seed &&                               // La canop??e est sup??rieure ?? un seuil pour ce pixel
                px.z > mhCrown*th_crown &&                            // La canop??e est sup??rieure ?? un  autre seuil pour ce pixel
                px.z <= hSeed+hSeed*0.05 &&                           // La canop??e est inf??rieure ?? un seuil pour ce pixel
                abs(seed.x-px.x) < DIST &&                              // Le pixel n'est pas trop loin du maximum local sur x
                abs(seed.y-px.y) < DIST &&                              // Le pixel n'est pas trop loin du maximum local sur y
                Region(px.x, px.y) == 0;

              if(expend)                                                // The pixel in part of the region
              {
                Regiontemp(px.x, px.y) = Region(r,k);                   // Add the pixel to the region
                npixel[id]++;
                sum_height[id] += Image(px.x, px.y);                    // Update the sum of the height of the region
                grown = true;
              }
            }
          }
        }
      }
    }

    std::copy( Regiontemp.begin(), Regiontemp.end(), Region.begin() );
  }

  return(Region);
}
