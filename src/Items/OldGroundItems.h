#pragma once

#include "Items/OldItem.h"
#include "Integrations/PluginServer.h"

namespace QuickLoot::Items
{
	using Element = QuickLoot::Integrations::TakenHandler::Element;

	class OldGroundItems final :
		public OldItem
	{
	private:
		using super = OldItem;

	public:
		OldGroundItems() = delete;
		OldGroundItems(const OldGroundItems&) = delete;
		OldGroundItems(OldGroundItems&&) = default;

		 OldGroundItems(std::ptrdiff_t a_count, bool a_stealing, std::vector<RE::ObjectRefHandle> a_items) :
			super(a_count, a_stealing, { a_items.data(), a_items.size() }),
			_items(std::move(a_items))
		{}

		~OldGroundItems() = default;

		OldGroundItems& operator=(const OldGroundItems&) = delete;
		OldGroundItems& operator=(OldGroundItems&&) = default;

	protected:
		 void DoTake(RE::Actor& a_dst, std::ptrdiff_t a_count) override
		{
			auto toRemove = std::clamp<std::ptrdiff_t>(a_count, 0, Count());
			if (toRemove <= 0) {
				assert(false);
				return;
			}

			std::vector<Element> elements;
			for (auto& handle : _items) {
				auto item = handle.get();
				if (item) {
					const auto xCount = std::clamp<std::ptrdiff_t>(item->extraList.GetCount(), 1, toRemove);
					a_dst.PickUpObject(item.get(), static_cast<std::int32_t>(xCount), false, true);
					toRemove -= xCount;
					elements.push_back({ item.get(), static_cast<std::int32_t>(xCount) });
					if (toRemove <= 0) {
						break;
					}
				}
			}
			QuickLoot::Integrations::PluginServer::HandleOnTaken(QuickLoot::Integrations::PluginServer::CreateOnTakenEvent(&a_dst, &elements));
		}

	private:
		std::vector<RE::ObjectRefHandle> _items;
	};
}
