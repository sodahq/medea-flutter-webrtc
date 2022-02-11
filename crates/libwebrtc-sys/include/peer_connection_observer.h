#pragma once

#include <functional>
#include <optional>

#include "api/peer_connection_interface.h"
#include "rust/cxx.h"

namespace bridge {

struct DynPeerConnectionEventsHandler;
struct DynSetDescriptionCallback;
struct DynCreateSdpCallback;

}  // namespace bridge

namespace observer {

// `PeerConnectionObserver` propagating events to the Rust side.
class PeerConnectionObserver : public webrtc::PeerConnectionObserver {
public:
  // Creates a new `PeerConnectionObserver`.
  PeerConnectionObserver(rust::Box<bridge::DynPeerConnectionEventsHandler> cb);

  // Called when a new ICE candidate has been discovered.
  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;

  // Called when an ICE candidate failed.
  void OnIceCandidateError(const std::string& address,
                           int port,
                           const std::string& url,
                           int error_code,
                           const std::string& error_text) override;

  // Called when the ICE candidates have been removed.
  void OnIceCandidatesRemoved(
      const std::vector<cricket::Candidate>& candidates) override;

  // Called when the `SignalingState` changes.
  void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState new_state) override;

  // Called any time the standards-compliant `IceConnectionState` changes.
  void OnStandardizedIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState new_state) override;

  // Called any time the `PeerConnectionState` changes.
  void OnConnectionChange(
      webrtc::PeerConnectionInterface::PeerConnectionState new_state) override;

  // Called when an ICE connection receiving status changes.
  void OnIceConnectionReceivingChange(bool receiving) override;

  // Called when the `IceGatheringState` changes.
  void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state) override;

  // Called when the selected candidate pair for an ICE connection changes.
  void OnIceSelectedCandidatePairChanged(
      const cricket::CandidatePairChangeEvent& event) override;

  // Called when a remote peer opens a data channel.
  void OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;

  // Used to fire spec-compliant `onnegotiationneeded` events, which should only
  // fire when the Operations Chain is empty. The observer is responsible for
  // queuing a task (e.g. Chromium: jump to main thread) to maybe fire the
  // event. The event identified using `event_id` must only fire if
  // `PeerConnection::ShouldFireNegotiationNeededEvent()` returns `true` since
  // it's possible for the event to become invalidated by operations
  // subsequently chained.
  void OnNegotiationNeededEvent(uint32_t event_id) override;

  // Called when a receiver and its track are created.
  //
  // NOTE: Called with both "Plan B" and "Unified Plan" semantics. "Unified
  //       Plan" users should prefer `OnTrack`, `OnAddTrack` is only called as
  //       backwards compatibility (and is called in the exact same situations
  //       as `OnTrack`).
  void OnAddTrack(
      rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
      const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams
  ) override;

  // Called when signaling indicates a transceiver will be receiving media from
  // a remote endpoint. It's fired during a call to `SetRemoteDescription`.
  //
  // The receiving track can be accessed by `transceiver->receiver()->track()`
  // and its associated streams by `transceiver->receiver()->streams()`.
  //
  // NOTE: Only called if "Unified Plan" semantics are specified.
  //       This behavior is specified in section 2.2.8.2.5 of the "Set the
  //       RTCSessionDescription" algorithm:
  //       https://w3c.github.io/webrtc-pc#set-description
  void OnTrack(
    rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override;

  // Called when signaling indicates that media will no longer be received on a
  // track.
  //
  // With "Plan B" semantics, the given receiver will be removed from the
  // `PeerConnection` and the muted track.
  //
  // With "Unified Plan" semantics, the receiver will remain, but the
  // transceiver will have its direction changed to either `sendonly` or
  // `inactive`.
  //
  // See more: https://w3c.github.io/webrtc-pc#process-remote-track-removal
  void OnRemoveTrack(
      rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override;

 private:
  // Rust side callback.
  rust::Box<bridge::DynPeerConnectionEventsHandler> cb_;
};

// `CreateSessionDescriptionObserver` propagating completion result to the Rust
// side.
class CreateSessionDescriptionObserver
    : public rtc::RefCountedObject<webrtc::CreateSessionDescriptionObserver> {
 public:
  // Creates a new `CreateSessionDescriptionObserver`.
  CreateSessionDescriptionObserver(rust::Box<bridge::DynCreateSdpCallback> cb);

  // Called when a `CreateOffer` or a `CreateAnswer` operation succeeds.
  void OnSuccess(webrtc::SessionDescriptionInterface* desc);

  // Called when a `CreateOffer` or a `CreateAnswer` operation fails.
  void OnFailure(webrtc::RTCError error);

 private:
  // Rust side callback.
  std::optional<rust::Box<bridge::DynCreateSdpCallback>> cb_;
};

// `SetLocalDescriptionObserverInterface` propagating completion result to the
// Rust side.
class SetLocalDescriptionObserver
    : public rtc::RefCountedObject<
        webrtc::SetLocalDescriptionObserverInterface> {
 public:
  // Creates a new `SetLocalDescriptionObserver`.
  SetLocalDescriptionObserver(rust::Box<bridge::DynSetDescriptionCallback> cb);

  // Called when a `SetLocalDescription` completes.
  void OnSetLocalDescriptionComplete(webrtc::RTCError error);

 private:
  // Rust side callback.
  std::optional<rust::Box<bridge::DynSetDescriptionCallback>> cb_;
};

// `SetRemoteDescriptionObserver` propagating completion result to the Rust
// side.
class SetRemoteDescriptionObserver
    : public rtc::RefCountedObject<
        webrtc::SetRemoteDescriptionObserverInterface> {
 public:
  // Creates a new `SetRemoteDescriptionObserver`.
  SetRemoteDescriptionObserver(rust::Box<bridge::DynSetDescriptionCallback> cb);

  // Called when a `SetRemoteDescription` completes.
  void OnSetRemoteDescriptionComplete(webrtc::RTCError error);

 private:
  // Rust side callback.
  std::optional<rust::Box<bridge::DynSetDescriptionCallback>> cb_;
};

}  // namespace observer