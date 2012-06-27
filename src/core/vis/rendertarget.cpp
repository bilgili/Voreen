#include "voreen/core/vis/rendertarget.h"

namespace voreen {

RenderTarget::RenderTarget(TextureContainer* const tc, const int textureID)
    : tc_(tc),
    textureID_(textureID),
    isReAssignable_(true)
{
}

RenderTarget::RenderTarget(const RenderTarget& other) {
    copy(other);
}

RenderTarget& RenderTarget::operator =(const RenderTarget& other) {
    copy(other);
    return (*this);
}

bool RenderTarget::operator >(const RenderTarget& other) const {
    if (tc_ == other.tc_)
        return (textureID_ > other.textureID_);
    return true;
}

// private methods
//

void RenderTarget::copy(const RenderTarget& other) {
    tc_ = other.tc_;
    textureID_ = other.textureID_;
    isReAssignable_ = other.isReAssignable_;
}

}   // namespace
