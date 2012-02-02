#include "mints.h"

using namespace boost;
using namespace psi;

IntegralFactory::IntegralFactory(boost::shared_ptr<BasisSet> bs1,
                                 boost::shared_ptr<BasisSet> bs2,
                                 boost::shared_ptr<BasisSet> bs3,
                                 boost::shared_ptr<BasisSet> bs4)
{
    set_basis(bs1, bs2, bs3, bs4);
}

IntegralFactory::IntegralFactory(boost::shared_ptr<BasisSet> bs1, boost::shared_ptr<BasisSet> bs2)
{
    set_basis(bs1, bs2, bs1, bs2);
}

IntegralFactory::IntegralFactory(boost::shared_ptr<BasisSet> bs1)
{
    set_basis(bs1, bs1, bs1, bs1);
}

IntegralFactory::~IntegralFactory()
{

}

boost::shared_ptr<BasisSet> IntegralFactory::basis1() const
{
    return bs1_;
}

boost::shared_ptr<BasisSet> IntegralFactory::basis2() const
{
    return bs2_;
}

boost::shared_ptr<BasisSet> IntegralFactory::basis3() const
{
    return bs3_;
}

boost::shared_ptr<BasisSet> IntegralFactory::basis4() const
{
    return bs4_;
}

void IntegralFactory::set_basis(boost::shared_ptr<BasisSet> bs1, boost::shared_ptr<BasisSet> bs2,
                boost::shared_ptr<BasisSet> bs3, boost::shared_ptr<BasisSet> bs4)
{
    bs1_ = bs1;
    bs2_ = bs2;
    bs3_ = bs3;
    bs4_ = bs4;

    // Find the max am
    boost::shared_ptr<BasisSet> max12(bs1_->max_am() > bs2_->max_am() ? bs1_ : bs2_);
    boost::shared_ptr<BasisSet> max34(bs3_->max_am() > bs4_->max_am() ? bs3_ : bs4_);
    boost::shared_ptr<BasisSet> max1234(max12->max_am() > max34->max_am() ? max12 : max34);

    init_spherical_harmonics(max1234->max_am());
}

OneBodyAOInt* IntegralFactory::ao_overlap(int deriv)
{
    return new OverlapInt(spherical_transforms_, bs1_, bs2_, deriv);
}

OneBodySOInt* IntegralFactory::so_overlap(int deriv)
{
    boost::shared_ptr<OneBodyAOInt> ao_int(ao_overlap(deriv));
    return new OneBodySOInt(ao_int, this);
}

ThreeCenterOverlapInt* IntegralFactory::overlap_3c()
{
    return new ThreeCenterOverlapInt(spherical_transforms_, bs1_, bs2_, bs3_);
}

OneBodyAOInt* IntegralFactory::ao_kinetic(int deriv)
{
    return new KineticInt(spherical_transforms_, bs1_, bs2_, deriv);
}

OneBodySOInt* IntegralFactory::so_kinetic(int deriv)
{
    boost::shared_ptr<OneBodyAOInt> ao_int(ao_kinetic(deriv));
    return new OneBodySOInt(ao_int, this);
}

OneBodyAOInt* IntegralFactory::ao_potential(int deriv)
{
    return new PotentialInt(spherical_transforms_, bs1_, bs2_, deriv);
}

OneBodySOInt* IntegralFactory::so_potential(int deriv)
{
    boost::shared_ptr<OneBodyAOInt> ao_int(ao_potential(deriv));
    return new PotentialSOInt(ao_int, this);
}

OneBodyAOInt* IntegralFactory::ao_pseudospectral(int deriv)
{
    return new PseudospectralInt(spherical_transforms_, bs1_, bs2_, deriv);
}

OneBodySOInt* IntegralFactory::so_pseudospectral(int deriv)
{
    boost::shared_ptr<OneBodyAOInt> ao_int(ao_pseudospectral(deriv));
    return new OneBodySOInt(ao_int, this);
}

OneBodyAOInt* IntegralFactory::electrostatic()
{
    return new ElectrostaticInt(spherical_transforms_, bs1_, bs2_, 0);
}

OneBodyAOInt* IntegralFactory::ao_dipole(int deriv)
{
    return new DipoleInt(spherical_transforms_, bs1_, bs2_, deriv);
}

OneBodySOInt* IntegralFactory::so_dipole(int deriv)
{
    boost::shared_ptr<OneBodyAOInt> ao_int(ao_dipole(deriv));
    return new OneBodySOInt(ao_int, this);
}

OneBodyAOInt* IntegralFactory::ao_nabla(int deriv)
{
    return new NablaInt(spherical_transforms_, bs1_, bs2_, deriv);
}

OneBodySOInt* IntegralFactory::so_nabla(int deriv)
{
    boost::shared_ptr<OneBodyAOInt> ao_int(ao_nabla(deriv));
    return new OneBodySOInt(ao_int, this);
}

OneBodyAOInt* IntegralFactory::ao_angular_momentum(int deriv)
{
    return new AngularMomentumInt(spherical_transforms_, bs1_, bs2_, deriv);
}

OneBodySOInt* IntegralFactory::so_angular_momentum(int deriv)
{
    boost::shared_ptr<OneBodyAOInt> ao_int(ao_angular_momentum(deriv));
    return new OneBodySOInt(ao_int, this);
}

OneBodyAOInt* IntegralFactory::ao_quadrupole()
{
    return new QuadrupoleInt(spherical_transforms_, bs1_, bs2_);
}

OneBodySOInt* IntegralFactory::so_quadrupole()
{
    boost::shared_ptr<OneBodyAOInt> ao_int(ao_quadrupole());
    return new OneBodySOInt(ao_int, this);
}

OneBodyAOInt* IntegralFactory::ao_traceless_quadrupole()
{
    return new TracelessQuadrupoleInt(spherical_transforms_, bs1_, bs2_);
}

OneBodySOInt* IntegralFactory::so_traceless_quadrupole()
{
    boost::shared_ptr<OneBodyAOInt> ao_int(ao_traceless_quadrupole());
    return new OneBodySOInt(ao_int, this);
}

OneBodyAOInt* IntegralFactory::electric_field()
{
    return new ElectricFieldInt(spherical_transforms_, bs1_, bs2_);
}

TwoBodyAOInt* IntegralFactory::eri(int deriv, double schwarz)
{
    return new ERI(this, deriv, schwarz);
}

TwoBodyAOInt* IntegralFactory::erf_eri(double omega, int deriv, double schwarz)
{
    return new ErfERI(omega, this, deriv, schwarz);
}

TwoBodyAOInt* IntegralFactory::erf_complement_eri(double omega, int deriv, double schwarz)
{
    return new ErfComplementERI(omega, this, deriv, schwarz);
}

void IntegralFactory::init_spherical_harmonics(int max_am)
{
    spherical_transforms_.clear();
    ispherical_transforms_.clear();

    for (int i=0; i<=max_am; ++i) {
        spherical_transforms_.push_back(SphericalTransform(i));
        ispherical_transforms_.push_back(ISphericalTransform(i));
    }
}

AOShellCombinationsIterator IntegralFactory::shells_iterator()
{
    return AOShellCombinationsIterator(bs1_, bs2_, bs3_, bs4_);
}

AOIntegralsIterator IntegralFactory::integrals_iterator(int p, int q, int r, int s)
{
    return AOIntegralsIterator(bs1_->shell(p), bs2_->shell(q), bs3_->shell(r), bs4_->shell(s));
}

CartesianIter* IntegralFactory::cartesian_iter(int l) const
{
    return new CartesianIter(l);
}

RedundantCartesianIter* IntegralFactory::redundant_cartesian_iter(int l) const
{
    return new RedundantCartesianIter(l);
}

RedundantCartesianSubIter* IntegralFactory::redundant_cartesian_sub_iter(int l) const
{
    return new RedundantCartesianSubIter(l);
}

ShellRotation IntegralFactory::shell_rotation(int am, SymmetryOperation &so, int pure) const
{
    ShellRotation r(am, so, this, pure);
    return r;
}

SphericalTransformIter* IntegralFactory::spherical_transform_iter(int am, int inv, int subl) const
{
    if (subl != -1)
        throw NotImplementedException("IntegralFactory::spherical_transform_iter: Not implemented for subl != -1.");

    if (inv) {
        return new SphericalTransformIter(ispherical_transforms_[am]);
    }
    return new SphericalTransformIter(spherical_transforms_[am]);
}
