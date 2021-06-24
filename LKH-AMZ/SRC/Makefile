# CC = gcc
IDIR = INCLUDE
ODIR = OBJ
CFLAGS = -O3 -Wall -g -I$(IDIR)

_DEPS = Hashing.h Heap.h LKH.h Segment.h

DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = Activate.o AddCandidate.o AddTourCandidates.o                   \
       AdjustCandidateSet.o AllocateStructures.o Ascent.o              \
       Between_SL.o CandidateReport.o                                  \
       C.o ChooseInitialTour.o Connect.o CreateCandidateSet.o          \
       Distance.o eprintf.o FindTour.o                                 \
       FixedCandidates.o Flip.o Flip_SL.o Forbidden.o                  \
       fscanint.o GenerateCandidates.o GetTime.o                       \
       Hashing.o Heap.o Improvement.o IsCandidate.o                    \
       IsPossibleCandidate.o KSwapKick.o LinKernighan.o                \
       LKHmain.o Make3OptMove.o                                        \
       Minimum1TreeCost.o MinimumSpanningTree.o                        \
       NormalizeNodeList.o NormalizeSegmentList.o                      \
       Penalty.o printff.o PrepareKicking.o PrintParameters.o          \
       Random.o ReadLine.o                                             \
       ReadParameters.o ReadProblem.o RecordBestTour.o                 \
       RecordBetterTour.o RemoveFirstActive.o                          \
       ResetCandidateSet.o RestoreTour.o                               \
       SpecialMove.o StatusReport.o Statistics.o StoreTour.o           \
       SymmetrizeCandidateSet.o WriteTour.o

OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS) | $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: 
	all clean

all:
	$(MAKE) LKH

LKH: $(OBJ) $(DEPS)
	$(CC) -o ../LKH $(OBJ) $(CFLAGS) -lm

$(ODIR):
	mkdir $(ODIR)

clean:
	/bin/rm -f $(ODIR)/* ../LKH *~ ._* $(IDIR)/*~ $(IDIR)/._*
	/bin/rmdir $(ODIR)

