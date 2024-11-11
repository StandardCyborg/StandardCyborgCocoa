/**
 This is not 100% our code.
 We extracted the packing algorithm from MaxRectsBinPack.h
 in the repo:
 https://github.com/juj/RectangleBinPack/
 In the survey, it was found that this is packing algorithm was the fastest.
 
 The author of the code seems fine with assigning MIT license to the code:
 https://github.com/juj/RectangleBinPack/issues/18#issuecomment-460626460
 So this code is thus effectivelly considered to be under the MIT-license.
 
 Note, that we did some modifications to the original code.
 The biggest one being that we replaced all ints by floats.
 The original code did all packing on integer coordinates.
 
 MIT License
 
 Copyright (c) 2019 Jukka Jylänki, Standard Cyborg
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 
 */
#pragma once

#include <vector>


namespace rbp {
    
    
    struct RectSize
    {
        float width;
        float height;
    };
    
    struct Rect
    {
        float x;
        float y;
        float width;
        float height;
    };
    
    /// Performs a lexicographic compare on (rect short side, rect long side).
    /// @return -1 if the smaller side of a is shorter than the smaller side of b, 1 if the other way around.
    ///   If they are equal, the larger side length is used as a tie-breaker.
    ///   If the rectangles are of same size, returns 0.
    int CompareRectShortSide(const Rect &a, const Rect &b);
    
    /// Performs a lexicographic compare on (x, y, width, height).
    int NodeSortCmp(const Rect &a, const Rect &b);
    
    /// Returns true if a is contained in b.
    
    inline bool IsContainedIn(const Rect &a, const Rect &b)
    {
        return a.x >= b.x && a.y >= b.y
        && a.x+a.width <= b.x+b.width
        && a.y+a.height <= b.y+b.height;
    }

    
    class DisjointRectCollection
    {
    public:
        std::vector<Rect> rects;
        
        bool Add(const Rect &r)
        {
            // Degenerate rectangles are ignored.
            if (r.width == 0 || r.height == 0)
                return true;
            
            if (!Disjoint(r))
                return false;
            rects.push_back(r);
            return true;
        }
        
        void Clear()
        {
            rects.clear();
        }
        
        bool Disjoint(const Rect &r) const
        {
            // Degenerate rectangles are ignored.
            if (r.width == 0 || r.height == 0)
                return true;
            
            for(size_t i = 0; i < rects.size(); ++i)
                if (!Disjoint(rects[i], r))
                    return false;
            return true;
        }
        
        static bool Disjoint(const Rect &a, const Rect &b)
        {
            if (a.x + a.width <= b.x ||
                b.x + b.width <= a.x ||
                a.y + a.height <= b.y ||
                b.y + b.height <= a.y)
                return true;
            return false;
        }
    };
    
    
    
    
    /** MaxRectsBinPack implements the MAXRECTS data structure and different bin packing algorithms that
     use this structure. */
    class MaxRectsBinPack
    {
    public:
        /// Instantiates a bin of size (0,0). Call Init to create a new bin.
        MaxRectsBinPack();
        
        /// Instantiates a bin of the given size.
        /// @param allowFlip Specifies whether the packing algorithm is allowed to rotate the input rectangles by 90 degrees to consider a better placement.
        MaxRectsBinPack(float width, float height, bool allowFlip = true);
        
        /// (Re)initializes the packer to an empty bin of width x height units. Call whenever
        /// you need to restart with a new bin.
        void Init(float width, float height, bool allowFlip = true);
        
        /// Specifies the different heuristic rules that can be used when deciding where to place a new rectangle.
        enum FreeRectChoiceHeuristic
        {
            RectBestShortSideFit, ///< -BSSF: Positions the rectangle against the short side of a free rectangle into which it fits the best.
            RectBestLongSideFit, ///< -BLSF: Positions the rectangle against the long side of a free rectangle into which it fits the best.
            RectBestAreaFit, ///< -BAF: Positions the rectangle into the smallest free rect into which it fits.
            RectBottomLeftRule, ///< -BL: Does the Tetris placement.
            RectContactPointRule ///< -CP: Choosest the placement where the rectangle touches other rects as much as possible.
        };
        
        /// Inserts the given list of rectangles in an offline/batch mode, possibly rotated.
        /// @param rects The list of rectangles to insert. This vector will be destroyed in the process.
        /// @param dst [out] This list will contain the packed rectangles. The indices will not correspond to that of rects.
        /// @param method The rectangle placement rule to use when packing.
        bool Insert(const std::vector<RectSize> &rects, std::vector<Rect> &dst, FreeRectChoiceHeuristic method);
        
        /// Inserts a single rectangle into the bin, possibly rotated.
        Rect Insert(float width, float height, FreeRectChoiceHeuristic method);
        
        /// Computes the ratio of used surface area to the total bin area.
        float Occupancy() const;
        
    private:
        int binWidth;
        int binHeight;
        
        bool binAllowFlip;
        
        std::vector<Rect> usedRectangles;
        std::vector<Rect> freeRectangles;
        
        /// Computes the placement score for placing the given rectangle with the given method.
        /// @param score1 [out] The primary placement score will be outputted here.
        /// @param score2 [out] The secondary placement score will be outputted here. This isu sed to break ties.
        /// @return This struct identifies where the rectangle would be placed if it were placed.
        Rect ScoreRect(float width, float height, FreeRectChoiceHeuristic method, float &score1, float  &score2) const;
        
        /// Places the given rectangle into the bin.
        void PlaceRect(const Rect &node);
        
        /// Computes the placement score for the -CP variant.
        float  ContactPointScoreNode(float x, float y, float width, float height) const;
        
        Rect FindPositionForNewNodeBottomLeft(float width, float height, float  &bestY, float  &bestX) const;
        Rect FindPositionForNewNodeBestShortSideFit(float width, float height, float  &bestShortSideFit, float  &bestLongSideFit) const;
        Rect FindPositionForNewNodeBestLongSideFit(float width, float height, float  &bestShortSideFit, float  &bestLongSideFit) const;
        Rect FindPositionForNewNodeBestAreaFit(float width, float height, float  &bestAreaFit, float  &bestShortSideFit) const;
        Rect FindPositionForNewNodeContactPoint(float width, float height, float  &contactScore) const;
        
        /// @return True if the free node was split.
        bool SplitFreeNode(Rect freeNode, const Rect &usedNode);
        
        /// Goes through the free rectangle list and removes any redundant entries.
        void PruneFreeList();
    };
    
}
