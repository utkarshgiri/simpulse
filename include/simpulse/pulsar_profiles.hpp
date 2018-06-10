#ifndef _SIMPULSE_PULSAR_PROFILES_HPP
#define _SIMPULSE_PULSAR_PROFILES_HPP

#if (__cplusplus < 201103) && !defined(__GXX_EXPERIMENTAL_CXX0X__)
#error "This source file needs to be compiled with C++0x support (g++ -std=c++0x)"
#endif

#include <vector>
#include <memory>


namespace simpulse {
#if 0
}; // pacify emacs c-mode
#endif


// Defined in pulsar_phase_models.hpp
struct phase_model_base;


// -------------------------------------------------------------------------------------------------
//
// von_mises_profile
//
// This class represents a von Mises profile, which can be used to simulate pulsars.
// (Currently, the von Mises profile is the only periodic pulse profile implemented in simpulse,
// although in the future we might implement more profiles and define a 'pulsar_profile' base class.)
//
// Constructor syntax:
//
//    p = simpulse.von_mises_profile(duty_cycle, detrend, min_internal_nphi=0)
//
// where:
//
//
// In order to simulate a pulsar, you need two things: a phase model and a pulse profile.  Then, to do
// the simulation, you can call either profile.eval_integrated_samples() or profile.add_integrated_samples().
// These methods take the phase model as a parameter.
//
// By default, the profile is normalized so that its peak flux is equal to 1 (before applying the detrending subtraction).
// To change the normalization, you can pass an 'amplitude' parameter to most of the von_mises_profile methods.  You may
// find the methods profile.get_single_pulse_signal_to_noise() and profile.get_multi_pulse_signal_to_noise() useful when
// setting the amplitude.
//
// Mathematically, a profile is just a function rho(Phi) which gives the flux 'rho' as a function of pulse phase 'phi'.
// The von Mises profile is the functional form:
//
//       rho(phi) = exp[ -2 kappa sin(pi*phi)^2 ]
//
// where kappa is a narrowness parameter, related to the duty cycle by kappa = log(2) / (2 sin^2(pi*D/2)).


class von_mises_profile {
public:
    
    // The 'duty_cycle' is defined as D = (pulse full width at half maximum) / (pulse period).
    // A reasonable choice is D = 0.1 or so.
    //
    // If the boolean variable 'detrend' is true, then the mean will be subtracted from the profile.
    //
    // It is unlikely that you'll need to set the 'min_internal_nphi' constructor argument, which
    // changes the number of phase bins used internally to represent the pulse.  If set to zero, then
    // a reasonable default value will be chosen.

    von_mises_profile(double duty_cycle, bool detrend, int min_internal_nphi=0);

    const double duty_cycle;
    const bool detrend;
    const int internal_nphi;


    // These are the main routines used to simulate a pulsar in a regularly spaced sequence of time samples.
    // The 'out' argument should point to an array of length nt.
    //
    // The 't0' argument should be the _beginning_ of the first time sample, and 't1' should be the _end_
    // of the last time sample.  Thus, t1=t0+nt*dt, where dt is the length of a sample (not t1=t0+(nt-1)*dt).
    //
    // Reminder: if the 'detrend' flag was specified at construction, then the simulated flux will be detrended

    void eval_integrated_samples(double *out, double t0, double t1, ssize_t nt, const phase_model_base &pm, double amplitude=1.0) const;

    // void add_integrated_samples(double *out, double t0, double t1, ssize_t nt, const phase_model_base &pm, double amplitude=1.0) const;

    // Returns the instantaneous flux evaluated at pulse phase 'phi'.
    // Reminder: if the 'detrend' flag was specified at construction, then the simulated flux will be detrended.
    double point_eval(double phi, double amplitude=1.0) const;    


    // get_single_pulse_signal_to_noise(): returns the SNR of a single pulse.
    // get_multi_pulse_signal_to_noise(): returns the SNR of a pulse train.
    //
    // The SNR calculation accounts for finite time resolution (and detrending, if detrend=True was specified
    // at construction), and assumes amplitude=1.
    //
    // Strictly speaking, the return value is an approximation to the true SNR, which may slightly depend
    // on the exact arrival times of the pulses.
    //
    // The 'total_time' argument is the total duration of the pulse train.
    // The 'dt_sample' argument is the length of each time sample.
    // The 'pulse_freq' argument is the pulse frequency.
    // The 'sample_rms' argument is the RMS noise fluctuation in each time sample.

    double get_single_pulse_signal_to_noise(double dt_sample, double pulse_freq, double sample_rms=1.0) const;
    double get_multi_pulse_signal_to_noise(double total_time, double dt_sample, double pulse_freq, double sample_rms=1.0) const;

    double get_mean_flux() const { return mean_flux; }

    
    // Returns the Fourier transform of the profile
    //     rho_m = int_0^1 dphi rho(phi) e^{i m phi}.
    //
    // Note that rho_m is real, and rho_m = rho_{-m}, since the von Mises profile is symmetric.
    // The DC mode rho_0 will equal 'mean_flux' if detrend=False, or 0 if detrend=True.
    //
    // The return value is a 1D array of length 'nout'.  If nout=0 (the default), then it defaults to
    // (internal_nphi/2+10, the number of Fourier coefficients which are computed internally.  (If 'nout'
    // is larger than this, then the returned array is zero-padded.

    template<typename T> void get_profile_fft(T *out, int nout) const;


    // This method is intended for debugging (hence the "_slow"!)
    // Returns the average flux over phase (not time) interval [phi0, phi1].
    double eval_integrated_sample_slow(double phi0, double phi1, double amplitude=1.0) const;

protected:
    const int internal_nphi2;
    const double kappa;
    double mean_flux = 0.0;
    
    // Note: padded to length (internal_nphi+1), for internal convenience interpolating.
    std::vector<double> detrended_profile;
    std::vector<double> detrended_profile_antider;
    
    // Length internal_nphi2, normalized to profile_fft[0]=1.
    std::vector<double> profile_fft;

    mutable std::vector<double> phi_tmp;   // length (tmp_block_size + 1)
    const ssize_t phi_block_size = 1024;

    double _get_rho2(double dphi) const;
};


}  // namespace simpulse

#endif // _SIMPULSE_PULSAR_PROFILES_HPP