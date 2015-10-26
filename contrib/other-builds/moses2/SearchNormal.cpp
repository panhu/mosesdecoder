/*
 * SearchNormal.cpp
 *
 *  Created on: 25 Oct 2015
 *      Author: hieu
 */

#include <boost/foreach.hpp>
#include "SearchNormal.h"
#include "Stack.h"
#include "Manager.h"
#include "InputPaths.h"
#include "TargetPhrases.h"
#include "TargetPhrase.h"

using namespace std;

SearchNormal::SearchNormal(Manager &mgr, std::vector<Stack> &stacks)
:m_mgr(mgr)
,m_stacks(stacks)
{
	// TODO Auto-generated constructor stub

}

SearchNormal::~SearchNormal() {
	// TODO Auto-generated destructor stub
}

void SearchNormal::Decode(size_t stackInd)
{
  Stack &stack = m_stacks[stackInd];

  BOOST_FOREACH(const Hypothesis *hypo, stack) {
		Extend(*hypo);
  }
  DebugStacks();
}

void SearchNormal::Extend(const Hypothesis &hypo)
{
	const InputPaths &paths = m_mgr.GetInputPaths();

	BOOST_FOREACH(const InputPath &path, paths) {
		Extend(hypo, path);
	}
}

void SearchNormal::Extend(const Hypothesis &hypo, const InputPath &path)
{
	const Moses::Bitmap &bitmap = hypo.GetBitmap();
	const Moses::Range &hypoRange = hypo.GetRange();
	const Moses::Range &pathRange = path.GetRange();

	if (bitmap.Overlap(pathRange)) {
		return;
	}

	int distortion = abs((int)pathRange.GetStartPos() - (int)hypoRange.GetEndPos() - 1);
	if (distortion > 5) {
		return;
	}

	const Moses::Bitmap &newBitmap = m_mgr.GetBitmaps().GetBitmap(bitmap, pathRange);
	const std::vector<const TargetPhrases*> &tpsAllPt = path.GetTargetPhrases();

	for (size_t i = 0; i < tpsAllPt.size(); ++i) {
		const TargetPhrases *tps = tpsAllPt[i];
		if (tps) {
			Extend(hypo, *tps, pathRange, newBitmap);
		}
	}
}

void SearchNormal::Extend(const Hypothesis &hypo,
		const TargetPhrases &tps,
		const Moses::Range &pathRange,
		const Moses::Bitmap &newBitmap)
{
  BOOST_FOREACH(const TargetPhrase *tp, tps) {
	  Extend(hypo, *tp, pathRange, newBitmap);
  }
}

void SearchNormal::Extend(const Hypothesis &hypo,
		const TargetPhrase &tp,
		const Moses::Range &pathRange,
		const Moses::Bitmap &newBitmap)
{
	Hypothesis *newHypo = new (m_mgr.GetPool().Allocate<Hypothesis>()) Hypothesis(hypo, tp, pathRange, newBitmap);

	size_t numWordsCovered = newBitmap.GetNumWordsCovered();
	m_stacks[numWordsCovered].Add(newHypo);
}

void SearchNormal::DebugStacks() const
{
	  BOOST_FOREACH(const Stack &stack, m_stacks) {
		  cerr << stack.GetSize() << " ";
	  }
	  cerr << endl;
}

const Hypothesis *SearchNormal::GetBestHypothesis() const
{
	const Stack &lastStack = m_stacks.back();
	std::vector<const Hypothesis*> hypos(lastStack.begin(), lastStack.end());

	//sort


	const Hypothesis *best = hypos[0];
	return best;
}


