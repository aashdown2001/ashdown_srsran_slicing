
#include "resource_grid.h"

using namespace srsgnb;

cell_slot_resource_grid::cell_slot_resource_grid(const cell_configuration& cfg_) :
  cfg(cfg_), dl_prbs(cfg.nof_dl_prbs), ul_prbs(cfg.nof_ul_prbs)
{}

void cell_slot_resource_grid::slot_indication(slot_point sl)
{
  reset();
  slot = sl;
}

void cell_slot_resource_grid::reset()
{
  dl_prbs.reset();
  ul_prbs.reset();
  dl_grants = {};
  ul_grants = {};
}

/////////////////////////////////////////////////////////////////////////////

cell_resource_grid::cell_resource_grid(const cell_configuration& cfg_) : cfg(cfg_)
{
  for (unsigned i = 0; i < RESOURCE_GRID_SIZE; ++i) {
    slots_.emplace_back(cfg);
  }
}

void cell_resource_grid::slot_indication(slot_point sl_tx)
{
  srsran_sanity_check(not last_slot_ind.valid() or last_slot_ind + 1 == sl_tx, "Slot indication was skipped");
  if (srsran_unlikely(not last_slot_ind.valid())) {
    // First call to slot_indication. Set slot of all slot cell resource grids.
    for (unsigned i = 0; i < RESOURCE_GRID_SIZE; ++i) {
      slots_[(sl_tx + i).to_uint() % RESOURCE_GRID_SIZE].slot_indication(sl_tx + i);
    }
  }
  last_slot_ind = sl_tx;

  // Reset old slot state and set its new future slot.
  auto& old_slot_res = slots_[(last_slot_ind - 1).to_uint() % RESOURCE_GRID_SIZE];
  old_slot_res.slot_indication(old_slot_res.slot + RESOURCE_GRID_SIZE);
}
