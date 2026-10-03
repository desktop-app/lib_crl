#pragma once

namespace crl::details {

void async_plain(void (*callable)(void*), void *argument);

} // namespace crl::details
