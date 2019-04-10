#include "DebugDyninst.h"

DebugDyninst::DebugDyninst(std::shared_ptr<DyninstProcess> proc) : _proc(proc) { 
	_bmap.reset(new BinaryLocationIDMap());

}

DebugDyninst::~DebugDyninst() {

}

void DebugDyninst::InsertDebug(std::set<uint64_t> & alreadyProcessed) {
	std::shared_ptr<DynOpsClass> ops = _proc->ReturnDynOps();
	BPatch_object * libcuda = _proc->LoadLibrary(std::string("libcuda.so.1"));


	// Create the dyninst functions from all the functions in the process.
	std::vector<BPatch_function *> all_functions;
	BPatch_image * img = _proc->GetAddressSpace()->getImage();
	img->getProcedures(all_functions);
	std::shared_ptr<InstrimentationTracker> tracker(new InstrimentationTracker());

	for (auto i : all_functions) {
		//_dyninstFunctions.push_back(std::shared_ptr<DyninstFunction>(new DyninstFunction(_proc, i, tracker, _bmap)));	
		_dyninstFunctions[(uint64_t)i->getBaseAddr()] = std::shared_ptr<DyninstFunction>(new DyninstFunction(_proc, i, tracker, _bmap));
	}

	// Rectify function list to remove anything +0x8
	{
		std::set<uint64_t> removeList;
		for (auto i : _dyninstFunctions) {
			if (_dyninstFunctions.find(i.first - 8) != _dyninstFunctions.end())
				removeList.insert(i.first);
		}
		std::cerr << "[LoadStoreInstrimentation::InsertAnalysis] Removing " << removeList.size() << " duplicate functions without premable" << std::endl;
		for (auto i : removeList)
			_dyninstFunctions.erase(i);
	}

	for (auto i : _dyninstFunctions) {
		if (i.first != alreadyProcessed){
			i.second->InsertLoadStoreAnalysis();
			alreadyProcessed.insert(i.first);
		}
	}

	// Print Debug Info
	PrintDebug(recs);

	_bmap->DumpLocationMap(std::string("DIOGENES_BINMAP.txt"));
}

void DebugDyninst::PrintDebug(StackRecMap & recs) {

	InstStats stats;
	stats.callTracedInsts = 0;
	stats.lsInsts = 0;
	std::ofstream t;
	t.open("DIOGENES_LSDEBUG.txt", std::ofstream::out);
	t << "Printing tracing stack" << std::endl;
	for (auto & i : recs) {
		i.second.PrintStack(t);
	}
	t << "\n";

	for (auto i : _dyninstFunctions) {
		t << i.second->PrintInst(stats);
	}

	t << "\n\nTotal Inst Points: " << stats.callTracedInsts + stats.lsInsts << std::endl;
	std::cout << "\n\nTotal Inst Points: " << stats.callTracedInsts + stats.lsInsts << std::endl;
	t << "Total CallTrace Points: " << stats.callTracedInsts << std::endl;
	std::cout << "Total CallTrace Points: " << stats.callTracedInsts << std::endl;
	t << "Calltraced Instructions: ";
	std::cout << "Calltraced Instructions: ";
	for (auto i : stats.ct_instNames){
		t << i << " ";
		std::cout << i << " ";
	}
	t << "\n";
	std::cout << "\n";

	t << "Total LS Points: " << stats.lsInsts << std::endl;
	std::cout << "Total LS Points: " << stats.lsInsts << std::endl;
	t << "LS Instructions: ";
	std::cout << "LS Instructions: ";
	for (auto i : stats.ls_instNames){
		t << i << " ";
		std::cout << i << " ";
	}
	t << "\n";
	std::cout << "\n";
	
	t.close();
}